#include <mm/mmu.h>

#include <stddef.h>

#include <asm/mmu.h>
#include <asm/page.h>

const mmu_alloc_ops_t* mmu_alloc_ops = NULL;

mmu_err_t mmu_init(struct mmu_space_s* space, const mmu_alloc_ops_t* alloc_ops)
{
  if(!space || !alloc_ops)
    return MMU_ERR_BAD_ARG;
    
  mmu_alloc_ops = alloc_ops;

  return arch_mmu_init(space);
}

mmu_err_t mmu_set_alloc_ops(const mmu_alloc_ops_t* alloc_ops)
{
  if(!alloc_ops)
    return MMU_ERR_BAD_ARG;
  mmu_alloc_ops = alloc_ops;
  return MMU_OK;
}

mmu_err_t mmu_map(struct mmu_space_s* space, virt_addr_t vaddr, phys_addr_t paddr, size_t size, mmu_flags_t flags)
{
  if(!space)
    return MMU_ERR_BAD_ARG;

  return arch_mmu_map(space, vaddr, paddr, size, flags);
}

mmu_err_t mmu_unmap(struct mmu_space_s* space, virt_addr_t vaddr, size_t size)
{
  if(!space)
    return MMU_ERR_BAD_ARG;

  return arch_mmu_unmap(space, vaddr, size);
}

mmu_err_t mmu_tlb_invalidate_page(struct mmu_space_s* space, virt_addr_t vaddr)
{
  if(!space)
    return MMU_ERR_BAD_ARG;

  return arch_mmu_tlb_invalidate_page(space, vaddr);
}

mmu_err_t mmu_tlb_flush(struct mmu_space_s* space)
{
  if(!space)
    return MMU_ERR_BAD_ARG;

  return arch_mmu_tlb_flush(space);
}
