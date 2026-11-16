#include <mm/vmalloc.h>

#include <asm/cpu.h>
#include <asm/page.h>
#include <asm/vmalloc.h>

#include <mm/mm.h>
#include <mm/mmu.h>
#include <lib/memory.h>

static void* vmap_internal(page_t** pages, size_t page_count, mmu_flags_t mmu_flags, vma_flags_t vma_flags, page_t* page_array_storage, uint8_t page_array_order, uint8_t owns_pages);
static void* vmalloc_order(size_t size, mmu_flags_t mmu_flags, uint8_t order);
static void rollback_mapping(virt_addr_t start, size_t size, uint8_t allow_partial);

void vmalloc_init(void) { }

void* vmap(page_t** pages, size_t page_count, mmu_flags_t mmu_flags, vma_flags_t vma_flags)
{
  return vmap_internal(pages, page_count, mmu_flags, vma_flags, NULL, 0, 0);
}

static void* vmap_internal(page_t** pages, size_t page_count, mmu_flags_t mmu_flags, vma_flags_t vma_flags, page_t* page_array_storage, uint8_t page_array_order, uint8_t owns_pages)
{
  if (!pages || page_count == 0 || (vma_flags & (VMA_FLAG_VMALLOC | VMA_FLAG_FREEING)))
    return NULL;

  if (page_count > ((VMALLOC_LIMIT - VMALLOC_START) >> PAGE_SHIFT))
    return NULL;

  const range_t vmalloc_range = {
    .start = VMALLOC_START,
    .end = VMALLOC_LIMIT - 1
  };

  size_t aligned_size = page_count << PAGE_SHIFT;
  vma_node_t* vma = vma_alloc_region(kernel_mm.vma, aligned_size, PAGE_SIZE, vmalloc_range);
  if (!vma)
    return NULL;

  size_t i = 0;
  while (i < page_count)
  {
    page_t* head = pages[i];
    if (!head || head->state != PAGE_STATE_ALLOCATED_HEAD ||
        head->order > MAX_PAGE_ORDER || head->compound_head != head)
    {
      rollback_mapping(vma->range.start, i << PAGE_SHIFT, 0);
      if (vma_free_region(kernel_mm.vma, vma) != 0)
        local_safe_halt();
      return NULL;
    }

    size_t block_pages = 1UL << head->order;
    if (block_pages > page_count - i)
    {
      rollback_mapping(vma->range.start, i << PAGE_SHIFT, 0);
      if (vma_free_region(kernel_mm.vma, vma) != 0)
        local_safe_halt();
      return NULL;
    }

    pfn_t first_pfn = page_to_pfn(head);
    uint8_t valid_block = 1;
    for (size_t j = 1; j < block_pages; j++)
    {
      page_t* tail = pfn_to_page(first_pfn + j);
      if (!tail || tail->state != PAGE_STATE_ALLOCATED_TAIL || tail->compound_head != head)
      {
        valid_block = 0;
        break;
      }
    }

    if (!valid_block)
    {
      rollback_mapping(vma->range.start, i << PAGE_SHIFT, 0);
      if (vma_free_region(kernel_mm.vma, vma) != 0)
        local_safe_halt();
      return NULL;
    }

    virt_addr_t vaddr = vma->range.start + (i << PAGE_SHIFT);
    phys_addr_t paddr = PFN_PHYS(first_pfn);
    size_t block_size = block_pages << PAGE_SHIFT;

    if (mmu_map(kernel_mm.mmu, vaddr, paddr, block_size, mmu_flags) != MMU_OK)
    {
      rollback_mapping(vaddr, block_size, 1);
      rollback_mapping(vma->range.start, i << PAGE_SHIFT, 0);
      if (vma_free_region(kernel_mm.vma, vma) != 0)
        local_safe_halt();
      return NULL;
    }
    i += block_pages;
  }

  vma_flags_t stored_flags = vma_flags | VMA_FLAG_VMAP;
  if (owns_pages)
    stored_flags |= VMA_FLAG_VMALLOC;

  if (vma_set_mapping(kernel_mm.vma, vma, pages, page_count, stored_flags, page_array_storage, page_array_order) != 0)
  {
    rollback_mapping(vma->range.start, aligned_size, 0);
    if (vma_free_region(kernel_mm.vma, vma) != 0)
      local_safe_halt();
    return NULL;
  }

  return (void*)vma->range.start;
}

void* vmalloc(size_t size)
{
  if (size == 0 || size > (VMALLOC_LIMIT - VMALLOC_START))
    return NULL;

  mmu_flags_t mmu_flags = MMU_FLAG_READ | MMU_FLAG_WRITE;
  int order = get_order(size);
  if (order > 0)
  {
    void* ptr = vmalloc_order(size, mmu_flags, (uint8_t)order);
    if (ptr)
      return ptr;
  }

  return vmalloc_order(size, mmu_flags, 0);
}

void vfree(void* addr)
{
  if (!addr)
    return;

  virt_addr_t vaddr = (virt_addr_t)addr;
  if (vaddr < VMALLOC_START || vaddr >= VMALLOC_LIMIT)
    return;

  vma_node_t* vma = vma_claim_region(kernel_mm.vma, vaddr, VMA_FLAG_VMAP);
  if (!vma)
    return;

  size_t total_size = vma->nr_pages << PAGE_SHIFT;
  if (vma->nr_pages == 0 || (total_size >> PAGE_SHIFT) != vma->nr_pages)
    local_safe_halt();

  if (mmu_unmap(kernel_mm.mmu, vma->range.start, total_size) != MMU_OK)
    local_safe_halt();

  if (vma->flags & VMA_FLAG_VMALLOC)
  {
    size_t i = 0;
    while (i < vma->nr_pages)
    {
      page_t* head = vma->pages[i];
      if (!head || head->state != PAGE_STATE_ALLOCATED_HEAD || head->order > MAX_PAGE_ORDER)
        local_safe_halt();

      size_t block_pages = 1UL << head->order;
      if (block_pages > vma->nr_pages - i)
        local_safe_halt();

      free_pages(head, head->order);
      i += block_pages;
    }

    if (!vma->page_array_storage || vma->page_array_order > MAX_PAGE_ORDER)
      local_safe_halt();
    free_pages(vma->page_array_storage, vma->page_array_order);
  }

  if (vma_free_claimed_region(kernel_mm.vma, vma) != 0)
    local_safe_halt();
}

static void* vmalloc_order(size_t size, mmu_flags_t mmu_flags, uint8_t order)
{
  if (size == 0 || size > (VMALLOC_LIMIT - VMALLOC_START))
    return NULL;

  size_t block_size = order_to_size(order);
  if (block_size < PAGE_SIZE)
    return NULL;

  size_t aligned_size = ALIGN_UP(size, block_size);
  if (aligned_size < size || aligned_size > (VMALLOC_LIMIT - VMALLOC_START))
    return NULL;

  size_t total_pages = aligned_size >> PAGE_SHIFT;
  if (total_pages > SIZE_MAX / sizeof(page_t*))
    return NULL;

  size_t page_array_bytes = total_pages * sizeof(page_t*);
  int page_array_order = get_order(page_array_bytes);
  if (page_array_order < 0)
    return NULL;

  page_t* page_array_storage = alloc_pages(ZONE_NORMAL, (uint8_t)page_array_order);
  if (!page_array_storage)
    return NULL;

  page_t** page_array = __va(PFN_PHYS(page_to_pfn(page_array_storage)));
  memset(page_array, 0, page_array_bytes);

  size_t pages_per_block = 1UL << order;
  size_t block_count = aligned_size / block_size;
  size_t allocated_blocks = 0;
  for (; allocated_blocks < block_count; allocated_blocks++)
  {
    page_t* head = alloc_pages(ZONE_NORMAL, order);
    if (!head)
      break;

    page_array[allocated_blocks * pages_per_block] = head;
  }

  if (allocated_blocks != block_count)
  {
    for (size_t i = 0; i < allocated_blocks; i++)
      free_pages(page_array[i * pages_per_block], order);

    free_pages(page_array_storage, (uint8_t)page_array_order);
    return NULL;
  }

  void* addr = vmap_internal(page_array, total_pages, mmu_flags, 0, page_array_storage, (uint8_t)page_array_order, 1);
  if (!addr)
  {
    for (size_t i = 0; i < block_count; i++)
      free_pages(page_array[i * pages_per_block], order);

    free_pages(page_array_storage, (uint8_t)page_array_order);
    return NULL;
  }

  return addr;
}

static void rollback_mapping(virt_addr_t start, size_t size, uint8_t allow_partial)
{
  if (!size)
    return;

  mmu_err_t err = mmu_unmap(kernel_mm.mmu, start, size);
  if (err != MMU_OK && !(allow_partial && err == MMU_ERR_NOT_MAPPED))
    local_safe_halt();
}
