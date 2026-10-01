#ifndef ASM_MMU_H
#define ASM_MMU_H

#include <mm/mmu.h>
#include <asm/paging.h>

typedef struct mmu_space_s {
  page_directory_t page_directory;
  phys_addr_t cr3;

  uint8_t is_kernel;
} mmu_space_t;

extern mmu_err_t arch_mmu_early_init(struct mmu_space_s* space);
extern mmu_err_t arch_mmu_patch_early_init(struct mmu_space_s* space, const mmu_alloc_ops_t* new_alloc_ops);
extern mmu_err_t arch_mmu_map(struct mmu_space_s* space, virt_addr_t vaddr, phys_addr_t paddr, size_t size, mmu_flags_t flags);

#endif
