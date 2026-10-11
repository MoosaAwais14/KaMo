#ifndef ASM_MMU_H
#define ASM_MMU_H

#include <stddef.h>

#include <mm/mmu_types.h>
#include <asm/paging.h>

typedef struct arch_mmu_space_s {
  page_directory_t page_directory;
  phys_addr_t cr3;
} arch_mmu_space_t;

extern mmu_err_t arch_mmu_init(arch_mmu_space_t* space);
extern mmu_err_t arch_mmu_preallocate_page_table(arch_mmu_space_t* space, virt_addr_t vaddr);
extern mmu_err_t arch_mmu_map(arch_mmu_space_t* space, virt_addr_t vaddr, phys_addr_t paddr, size_t size, mmu_flags_t flags);
extern mmu_err_t arch_mmu_unmap(arch_mmu_space_t* space, virt_addr_t vaddr, size_t size);
extern mmu_err_t arch_mmu_tlb_invalidate_page(arch_mmu_space_t* space, virt_addr_t vaddr);
extern mmu_err_t arch_mmu_tlb_flush(arch_mmu_space_t* space);

#endif
