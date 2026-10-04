#ifndef ASM_FIXMAP_H
#define ASM_FIXMAP_H

#if !defined(__ASSEMBLER__) && !defined(LINKER_SCRIPT)

#include <asm/paging.h>
#include <kernel/cpu.h>
#include <mm/mmu.h>

#define FIXMAP_SLOTS_PER_CPU 4
#define FIXMAP_SIZE_SHIFT   PAGE_SHIFT
#define FIXMAP_SIZE         (1UL << FIXMAP_SIZE_SHIFT)
#define FIXMAP_BASE         DIRECT_MAP_LIMIT
#define FIXMAP_CPU_STRIDE   (FIXMAP_SLOTS_PER_CPU * FIXMAP_SIZE)
#define FIXMAP_WINDOW_SIZE  (CPU_MAX * FIXMAP_CPU_STRIDE)

#define FIXMAP_SLOT_ADDR(cpu_id, slot_idx) \
	(FIXMAP_BASE + ((cpu_id) * FIXMAP_CPU_STRIDE) + ((slot_idx) * FIXMAP_SIZE))

extern mmu_err_t arch_fixmap_early_init(struct mmu_space_s* space);

extern mmu_err_t arch_fixmap_map(struct mmu_space_s* space, phys_addr_t paddr, mmu_flags_t flags, virt_addr_t* vaddr);
extern mmu_err_t arch_fixmap_unmap(struct mmu_space_s* space, virt_addr_t vaddr);

#endif

#endif
