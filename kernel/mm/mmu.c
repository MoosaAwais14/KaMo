#include <mm/mmu.h>

#include <stddef.h>

#include <asm/page.h>

const mmu_alloc_ops_t* mmu_alloc_ops = NULL;

void mmu_set_allocator(const mmu_alloc_ops_t* alloc_ops)
{
  mmu_alloc_ops = alloc_ops;
}

mmu_err_t mmu_init(mmu_space_t* space)
{
  if(!space || !mmu_alloc_ops)
    return MMU_ERR_BAD_ARG;

  return arch_mmu_init(&space->arch);
}

mmu_err_t mmu_map(mmu_space_t* space, virt_addr_t vaddr, phys_addr_t paddr, size_t size, mmu_flags_t flags)
{
  if(!space)
    return MMU_ERR_BAD_ARG;
  
  unsigned long irq_flags = raw_spin_lock_irqsave(&space->rlock);
  mmu_err_t err = arch_mmu_map(&space->arch, vaddr, paddr, size, flags);
  raw_spin_unlock_irqrestore(&space->rlock, irq_flags);

  return err;
}

mmu_err_t mmu_unmap(mmu_space_t* space, virt_addr_t vaddr, size_t size)
{
  if(!space)
    return MMU_ERR_BAD_ARG;

  unsigned long irq_flags = raw_spin_lock_irqsave(&space->rlock);
  mmu_err_t err = arch_mmu_unmap(&space->arch, vaddr, size);
  raw_spin_unlock_irqrestore(&space->rlock, irq_flags);

  return err;
}

mmu_err_t mmu_tlb_invalidate_page(mmu_space_t* space, virt_addr_t vaddr)
{
  if(!space)
    return MMU_ERR_BAD_ARG;

  unsigned long irq_flags = raw_spin_lock_irqsave(&space->rlock);
  mmu_err_t err =  arch_mmu_tlb_invalidate_page(&space->arch, vaddr);
  raw_spin_unlock_irqrestore(&space->rlock, irq_flags);

  return err;
}

mmu_err_t mmu_tlb_flush(mmu_space_t* space)
{
  if(!space)
    return MMU_ERR_BAD_ARG;

  unsigned long irq_flags = raw_spin_lock_irqsave(&space->rlock);
  mmu_err_t err =   arch_mmu_tlb_flush(&space->arch);
  raw_spin_unlock_irqrestore(&space->rlock, irq_flags);

  return err;
}
