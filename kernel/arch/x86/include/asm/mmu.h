#ifndef ASM_MMU_H
#define ASM_MMU_H

#include <mm/mmu.h>
#include <asm/paging.h>

typedef struct mmu_space_s {
  page_directory_t page_directory;
  phys_addr_t cr3;

  uint8_t is_kernel;
} mmu_space_t;

extern mmu_err_t arch_mmu_init(struct mmu_space_s* space);
extern mmu_err_t arch_mmu_preallocate_page_table(struct mmu_space_s* space, virt_addr_t vaddr);
extern mmu_err_t arch_mmu_map(struct mmu_space_s* space, virt_addr_t vaddr, phys_addr_t paddr, size_t size, mmu_flags_t flags);
extern mmu_err_t arch_mmu_unmap(struct mmu_space_s* space, virt_addr_t vaddr, size_t size);
extern mmu_err_t arch_mmu_tlb_invalidate_page(struct mmu_space_s* space, virt_addr_t vaddr);
extern mmu_err_t arch_mmu_tlb_flush(struct mmu_space_s* space);

#endif
