#ifndef ASM_FIXMAP_H
#define ASM_FIXMAP_H

#if !defined(__ASSEMBLER__) && !defined(LINKER_SCRIPT)

#include <asm/page.h>
#include <kernel/cpu.h>
#include <mm/mmu.h>

#include <lib/memory.h>

#define FIXMAP_SLOTS_PER_CPU     4
#define FIXMAP_SLOT_SIZE_SHIFT  PAGE_SHIFT
#define FIXMAP_SLOT_SIZE        (1UL << FIXMAP_SLOT_SIZE_SHIFT)
#define FIXMAP_CPU_STRIDE       (FIXMAP_SLOTS_PER_CPU * FIXMAP_SLOT_SIZE)
#define FIXMAP_SIZE             (CPU_MAX * FIXMAP_CPU_STRIDE)
#define FIXMAP_BASE             ALIGN_DOWN((FIXMAP_LIMIT_END - FIXMAP_SIZE + 1), PAGE_SIZE_HUGE)
#define FIXMAP_END              (ALIGN_UP(FIXMAP_BASE + FIXMAP_SIZE, PAGE_SIZE_HUGE) - 1)

#define FIXMAP_SLOT_ADDR(cpu_id, slot_idx) \
	(FIXMAP_BASE + ((cpu_id) * FIXMAP_CPU_STRIDE) + ((slot_idx) * FIXMAP_SLOT_SIZE))

extern mmu_err_t arch_fixmap_init(arch_mmu_space_t* space);

extern mmu_err_t arch_fixmap_map(arch_mmu_space_t* space, phys_addr_t paddr, mmu_flags_t flags, virt_addr_t* vaddr);
extern mmu_err_t arch_fixmap_unmap(arch_mmu_space_t* space, virt_addr_t vaddr);

#endif

#endif
