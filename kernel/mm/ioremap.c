#include <mm/ioremap.h>

#include <asm/page.h>
#include <asm/vmalloc.h>

#include <lib/memory.h>
#include <mm/mm.h>
#include <mm/mmu.h>
#include <mm/vma.h>

mmu_err_t ioremap(phys_addr_t paddr, size_t size, mmu_flags_t flags, void** out_addr)
{
  if (!out_addr || !size)
    return MMU_ERR_BAD_ARG;

  const size_t offset = paddr & (PAGE_SIZE - 1);
  if (size > SIZE_MAX - offset)
    return MMU_ERR_BAD_ARG;

  const size_t span = offset + size;
  if (span > SIZE_MAX - (PAGE_SIZE - 1))
    return MMU_ERR_BAD_ARG;

  const size_t mapped_size = ALIGN_UP(span, PAGE_SIZE);
  const phys_addr_t mapped_paddr = ALIGN_DOWN(paddr, PAGE_SIZE);
  if (!mapped_size || (phys_addr_t)(mapped_size - 1) > UINT64_MAX - mapped_paddr)
    return MMU_ERR_BAD_ARG;

  const range_t vmalloc_range = {
    .start = VMALLOC_START,
    .end = VMALLOC_LIMIT - 1
  };

  vma_node_t* region = vma_alloc_region(kernel_mm.vma, mapped_size, PAGE_SIZE, vmalloc_range);
  if (!region)
    return MMU_ERR_NO_RESOURCES;

  if (vma_set_iomap(kernel_mm.vma, region) != 0)
  {
    if (vma_free_region(kernel_mm.vma, region) != 0)
      return MMU_ERR_NO_RESOURCES;
    return MMU_ERR_BAD_ARG;
  }

  mmu_err_t err = mmu_map(kernel_mm.mmu, region->range.start, mapped_paddr, mapped_size, flags);
  if (err != MMU_OK)
  {
    mmu_err_t unmap_err = mmu_unmap(kernel_mm.mmu, region->range.start, mapped_size);
    if (unmap_err != MMU_OK && unmap_err != MMU_ERR_NOT_MAPPED)
    {
      *out_addr = (void*)(region->range.start + offset);
      return unmap_err;
    }

    vma_node_t* claimed = vma_claim_region(kernel_mm.vma, region->range.start, VMA_FLAG_IOMAP);
    if (!claimed)
      return MMU_ERR_NO_RESOURCES;

    if (vma_free_claimed_region(kernel_mm.vma, claimed) != 0)
      return MMU_ERR_NO_RESOURCES;

    return err;
  }

  *out_addr = (void*)(region->range.start + offset);
  return MMU_OK;
}

mmu_err_t iounmap(void* addr)
{
  if (!addr)
    return MMU_ERR_BAD_ARG;

  virt_addr_t base = ALIGN_DOWN((virt_addr_t)addr, PAGE_SIZE);
  vma_node_t* region = vma_claim_region(kernel_mm.vma, base, VMA_FLAG_IOMAP);
  if (!region)
    return MMU_ERR_NOT_MAPPED;

  uint64_t mapped_length = range_len(&region->range);
  if (!mapped_length || mapped_length > SIZE_MAX)
    return MMU_ERR_BAD_ARG;

  mmu_err_t err = mmu_unmap(kernel_mm.mmu, region->range.start, (size_t)mapped_length);
  if (err != MMU_OK)
    return err;

  if (vma_free_claimed_region(kernel_mm.vma, region) != 0)
    return MMU_ERR_NO_RESOURCES;
  return MMU_OK;
}
