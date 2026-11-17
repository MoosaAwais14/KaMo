#include <mm/vmalloc.h>

#include <asm/cpu.h>
#include <asm/page.h>
#include <asm/vmalloc.h>

#include <mm/mm.h>
#include <mm/mmu.h>
#include <lib/memory.h>

static void* vmap_internal(page_t** pages, size_t page_count, mmu_flags_t mmu_flags, page_t* page_array_storage, uint8_t page_array_order, uint8_t owns_pages);
static void* vmalloc_order(size_t size, mmu_flags_t mmu_flags, uint8_t order);
static void rollback_mapping(virt_addr_t start, size_t size);
static vma_node_t* vmap_claim_and_unmap(void* addr, vma_flags_t required_flags);

void vmalloc_init(void) { }

void* vmap(page_t** pages, size_t page_count, mmu_flags_t mmu_flags)
{
  return vmap_internal(pages, page_count, mmu_flags, NULL, 0, 0);
}

int vunmap(void* addr)
{
  vma_node_t* vma = vmap_claim_and_unmap(addr, VMA_FLAG_VMAP);
  if (!vma)
    return -1;

  if (vma_free_claimed_region(kernel_mm.vma, vma) != 0)
    return 1;

  return 0;
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

int vfree(void* addr)
{
  vma_node_t* vma = vmap_claim_and_unmap(addr, VMA_FLAG_VMAP | VMA_FLAG_VMALLOC);
  if (!vma)
    return -1;

  size_t i = 0;
  while (i < vma->nr_pages)
  {
    page_t* head = vma->pages[i];
    if (!head || head->state != PAGE_STATE_ALLOCATED_HEAD || head->order > MAX_PAGE_ORDER)
      return 1;

    size_t block_pages = 1UL << head->order;
    if (block_pages > vma->nr_pages - i)
      return 2;

    free_pages(head, head->order);
    i += block_pages;
  }

  if (!vma->page_array_storage || vma->page_array_order > MAX_PAGE_ORDER)
    return 3;
  free_pages(vma->page_array_storage, vma->page_array_order);

  if (vma_free_claimed_region(kernel_mm.vma, vma) != 0)
    return 4;

  return 0;
}

static void* vmap_internal(page_t** pages, size_t page_count, mmu_flags_t mmu_flags, page_t* page_array_storage, uint8_t page_array_order, uint8_t owns_pages)
{
  if (!pages || page_count == 0)
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
      rollback_mapping(vma->range.start, i << PAGE_SHIFT);
      vma_free_region(kernel_mm.vma, vma);
      return NULL;
    }

    size_t block_pages = 1UL << head->order;
    if (block_pages > page_count - i)
    {
      rollback_mapping(vma->range.start, i << PAGE_SHIFT);
      vma_free_region(kernel_mm.vma, vma);
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
      rollback_mapping(vma->range.start, i << PAGE_SHIFT);
      vma_free_region(kernel_mm.vma, vma);
      return NULL;
    }

    virt_addr_t vaddr = vma->range.start + (i << PAGE_SHIFT);
    phys_addr_t paddr = PFN_PHYS(first_pfn);
    size_t block_size = block_pages << PAGE_SHIFT;

    if (mmu_map(kernel_mm.mmu, vaddr, paddr, block_size, mmu_flags) != MMU_OK)
    {
      rollback_mapping(vaddr, block_size);
      rollback_mapping(vma->range.start, i << PAGE_SHIFT);
      vma_free_region(kernel_mm.vma, vma);
      return NULL;
    }
    i += block_pages;
  }

  vma_flags_t stored_flags = VMA_FLAG_VMAP;
  if (owns_pages)
    stored_flags |= VMA_FLAG_VMALLOC;

  if (vma_set_page_backing(kernel_mm.vma, vma, pages, page_count, stored_flags, page_array_storage, page_array_order) != 0)
  {
    rollback_mapping(vma->range.start, aligned_size);
    vma_free_region(kernel_mm.vma, vma);
    return NULL;
  }

  return (void*)vma->range.start;
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

  void* addr = vmap_internal(page_array, total_pages, mmu_flags, page_array_storage, (uint8_t)page_array_order, 1);
  if (!addr)
  {
    for (size_t i = 0; i < block_count; i++)
      free_pages(page_array[i * pages_per_block], order);

    free_pages(page_array_storage, (uint8_t)page_array_order);
    return NULL;
  }

  return addr;
}

static void rollback_mapping(virt_addr_t start, size_t size)
{
  if (!size)
    return;
  mmu_unmap(kernel_mm.mmu, start, size);
}

static vma_node_t* vmap_claim_and_unmap(void* addr, vma_flags_t required_flags)
{
  if (!addr)
    return NULL;

  virt_addr_t vaddr = (virt_addr_t)addr;
  if (vaddr < VMALLOC_START || vaddr >= VMALLOC_LIMIT)
    return NULL;

  vma_node_t* vma = vma_claim_region(kernel_mm.vma, vaddr, required_flags);
  if (!vma)
    return NULL;

  if (!vma->nr_pages || vma->nr_pages > (SIZE_MAX >> PAGE_SHIFT))
    return NULL;

  size_t total_size = vma->nr_pages << PAGE_SHIFT;
  if (mmu_unmap(kernel_mm.mmu, vma->range.start, total_size) != MMU_OK)
    return NULL;

  return vma;
}
