#ifndef MM_MMU_H
#define MM_MMU_H

#include <stdint.h>
#include <stddef.h>

#include <kernel/address.h>

struct mmu_space_s;

typedef enum mmu_err_e
{
  MMU_OK,
  MMU_ERR_BAD_ARG,
  MMU_ERR_ALLOCATOR,

} mmu_err_t;

typedef struct mmu_alloc_ops_s
{
  phys_addr_t (*alloc_frame)(void* ctx);
  void (*free_frame)(void* ctx, phys_addr_t phys);
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

extern mmu_err_t mmu_early_init(struct mmu_space_s* space, const mmu_alloc_ops_t* alloc_ops);
extern mmu_err_t mmu_patch_early_init(struct mmu_space_s* space, const mmu_alloc_ops_t* new_alloc_ops);
extern mmu_err_t mmu_map(struct mmu_space_s* space, virt_addr_t vaddr, phys_addr_t paddr, size_t size, mmu_flags_t flags);

extern const mmu_alloc_ops_t* mmu_alloc_ops;

#endif
