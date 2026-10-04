#include <asm/fixmap.h>
#include <asm/mmu.h>
#include <asm/cpu.h>
#include <asm/cpu_arch.h>

#include <kernel/cpu.h>
#include <locking/spinlock.h>
#include <lib/memory.h>

struct fixmap_slots_s {
  uint8_t in_use;
  raw_spinlock_t rlock;
};

struct cpu_fixmap_s {
  struct fixmap_slots_s slots[FIXMAP_SLOTS_PER_CPU];
};

static struct cpu_fixmap_s cpu_fixmaps[CPU_MAX];

mmu_err_t arch_fixmap_early_init(struct mmu_space_s* space)
{
  if (!IS_ALIGNED(FIXMAP_BASE, PAGE_SIZE_HUGE) ||
    !FIXMAP_WINDOW_SIZE || FIXMAP_WINDOW_SIZE > PAGE_SIZE_HUGE)
    return MMU_ERR_BAD_ARG;

  for(size_t cpu_id = 0; cpu_id < CPU_MAX; cpu_id++)
  {
    struct cpu_fixmap_s* cpu_fixmap = &cpu_fixmaps[cpu_id];
    for(size_t slot_idx = 0; slot_idx < FIXMAP_SLOTS_PER_CPU; slot_idx++)
    {
      struct fixmap_slots_s* slot = &cpu_fixmap->slots[slot_idx];
      slot->in_use = 0;
      slot->rlock = RAW_SPINLOCK_UNLOCKED;
    }
  }

  return arch_mmu_preallocate_page_table(space, FIXMAP_BASE);
}

mmu_err_t arch_fixmap_map(struct mmu_space_s* space, phys_addr_t paddr, mmu_flags_t flags, virt_addr_t* vaddr)
{
  if (!space || !vaddr)
    return MMU_ERR_BAD_ARG;

  preempt_disable();
  if (read_cr3() != space->cr3) 
  {
    preempt_enable();
    return MMU_ERR_BAD_ARG;
  }

  uint32_t cpu_id = arch_cpu_current_id();
  if (cpu_id >= CPU_MAX) 
  {
    preempt_enable();
    return MMU_ERR_BAD_ARG;
  }

  struct cpu_fixmap_s* cpu_fixmap = &cpu_fixmaps[cpu_id];
  unsigned long irq_flags;
  for (size_t slot_idx = 0; slot_idx < FIXMAP_SLOTS_PER_CPU; slot_idx++)
  {
    struct fixmap_slots_s* slot = &cpu_fixmap->slots[slot_idx];
    irq_flags = raw_spin_lock_irqsave(&slot->rlock);

    if (slot->in_use)
    {
      raw_spin_unlock_irqrestore(&slot->rlock, irq_flags);
      continue;
    }

    virt_addr_t slot_vaddr = FIXMAP_SLOT_ADDR(cpu_id, slot_idx);
    slot->in_use = 1;
    raw_spin_unlock_irqrestore(&slot->rlock, irq_flags);

    mmu_err_t ret = mmu_map(space, slot_vaddr, paddr, FIXMAP_SIZE, flags);
    if (ret != MMU_OK)
    {
      irq_flags = raw_spin_lock_irqsave(&slot->rlock);
      slot->in_use = 0;
      raw_spin_unlock_irqrestore(&slot->rlock, irq_flags);
      preempt_enable();
      return ret;
    }

    *vaddr = slot_vaddr;
    return MMU_OK;
  }

  preempt_enable();
  return MMU_ERR_NO_RESOURCES;
}

mmu_err_t arch_fixmap_unmap(struct mmu_space_s* space, virt_addr_t vaddr)
{
  if (!space)
    return MMU_ERR_BAD_ARG;

  if (read_cr3() != space->cr3)
    return MMU_ERR_BAD_ARG;

  uint32_t cpu_id = arch_cpu_current_id();
  if (cpu_id >= CPU_MAX)
    return MMU_ERR_BAD_ARG;

  struct cpu_fixmap_s* cpu_fixmap = &cpu_fixmaps[cpu_id];
  for (size_t slot_idx = 0; slot_idx < FIXMAP_SLOTS_PER_CPU; slot_idx++)
  {
    if(FIXMAP_SLOT_ADDR(cpu_id, slot_idx) != vaddr)
      continue;

    struct fixmap_slots_s* slot = &cpu_fixmap->slots[slot_idx];
    unsigned long irq_flags = raw_spin_lock_irqsave(&slot->rlock);

    if (!slot->in_use) 
    {
      raw_spin_unlock_irqrestore(&slot->rlock, irq_flags);
      return MMU_ERR_NOT_MAPPED;
    }

    raw_spin_unlock_irqrestore(&slot->rlock, irq_flags);

    mmu_err_t ret = mmu_unmap(space, vaddr, FIXMAP_SIZE);
    if (ret != MMU_OK)
      return ret;

    irq_flags = raw_spin_lock_irqsave(&slot->rlock);
    slot->in_use = 0;
    raw_spin_unlock_irqrestore(&slot->rlock, irq_flags);
    preempt_enable();
    return MMU_OK;
  }

  return MMU_ERR_BAD_ARG;
}
