#ifndef MM_MMU_H
#define MM_MMU_H

#include <stdint.h>
#include <stddef.h>

#include <mm/mmu_types.h>
#include <asm/mmu.h>

#include <locking/spinlock.h>
#include <kernel/address.h>

typedef struct mmu_space_s {
  arch_mmu_space_t arch;
  raw_spinlock_t rlock;
  uint8_t is_kernel;
} mmu_space_t;

typedef struct mmu_alloc_ops_s {
    mmu_err_t (*alloc_page)(void *ctx, phys_addr_t* out);
    void (*release_page)(void *ctx, phys_addr_t paddr);
    void *ctx;
} mmu_alloc_ops_t;

extern mmu_err_t mmu_init(mmu_space_t* space, const mmu_alloc_ops_t* alloc_ops);
extern mmu_err_t mmu_map(mmu_space_t* space, virt_addr_t vaddr, phys_addr_t paddr, size_t size, mmu_flags_t flags);
extern mmu_err_t mmu_unmap(mmu_space_t* space, virt_addr_t vaddr, size_t size);
extern mmu_err_t mmu_tlb_invalidate_page(mmu_space_t* space, virt_addr_t vaddr);
extern mmu_err_t mmu_tlb_flush(mmu_space_t* space);

extern const mmu_alloc_ops_t* mmu_alloc_ops;

#endif
