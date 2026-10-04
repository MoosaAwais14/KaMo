#ifndef MM_MMU_H
#define MM_MMU_H

#include <stdint.h>
#include <stddef.h>

#include <kernel/address.h>

struct mmu_space_s;

typedef enum mmu_err_e
{
  MMU_OK = 0,
  MMU_ERR_BAD_ARG,
  MMU_ERR_ALLOCATOR,
  MMU_ERR_NOT_MAPPED,
  MMU_ERR_FIELD_MISMATCH,
  MMU_ERR_NO_RESOURCES
} mmu_err_t;

typedef struct mmu_frame_s {
    phys_addr_t phys;
    void *priv;
} mmu_frame_t;

typedef struct mmu_alloc_ops_s {
    mmu_err_t (*alloc_page)(void *ctx, phys_addr_t* out);
    void (*release_page)(void *ctx, phys_addr_t paddr);
    void *ctx;
} mmu_alloc_ops_t;

typedef uint64_t mmu_flags_t;

#define MMU_FLAG_WRITE       (1ull << 0)
#define MMU_FLAG_USER        (1ull << 1)
#define MMU_FLAG_EXEC        (1ull << 2)
#define MMU_FLAG_GLOBAL      (1ull << 3)

#define MMU_FLAG_CACHE_SHIFT 8
#define MMU_FLAG_CACHE_MASK  (3ull << MMU_FLAG_CACHE_SHIFT)
#define MMU_FLAG_CACHE_WB    (0ull << MMU_FLAG_CACHE_SHIFT)
#define MMU_FLAG_CACHE_WT    (1ull << MMU_FLAG_CACHE_SHIFT)
#define MMU_FLAG_CACHE_WC    (2ull << MMU_FLAG_CACHE_SHIFT)
#define MMU_FLAG_CACHE_UC    (3ull << MMU_FLAG_CACHE_SHIFT)

extern mmu_err_t mmu_init(struct mmu_space_s* space, const mmu_alloc_ops_t* alloc_ops);
extern mmu_err_t mmu_set_alloc_ops(const mmu_alloc_ops_t* alloc_ops);
extern mmu_err_t mmu_map(struct mmu_space_s* space, virt_addr_t vaddr, phys_addr_t paddr, size_t size, mmu_flags_t flags);
extern mmu_err_t mmu_unmap(struct mmu_space_s* space, virt_addr_t vaddr, size_t size);
extern mmu_err_t mmu_tlb_invalidate_page(struct mmu_space_s* space, virt_addr_t vaddr);
extern mmu_err_t mmu_tlb_flush(struct mmu_space_s* space);

extern const mmu_alloc_ops_t* mmu_alloc_ops;

#endif
