#ifndef MM_MMU_TYPES_H
#define MM_MMU_TYPES_H

#include <stdint.h>

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

typedef uint64_t mmu_flags_t;

#define MMU_FLAG_READ        (1ull << 0)
#define MMU_FLAG_WRITE       (1ull << 1)
#define MMU_FLAG_USER        (1ull << 2)
#define MMU_FLAG_EXEC        (1ull << 3)
#define MMU_FLAG_GLOBAL      (1ull << 4)

#define MMU_FLAG_CACHE_SHIFT 8
#define MMU_FLAG_CACHE_MASK  (3ull << MMU_FLAG_CACHE_SHIFT)
#define MMU_FLAG_CACHE_WB    (0ull << MMU_FLAG_CACHE_SHIFT)
#define MMU_FLAG_CACHE_WT    (1ull << MMU_FLAG_CACHE_SHIFT)
#define MMU_FLAG_CACHE_WC    (2ull << MMU_FLAG_CACHE_SHIFT)
#define MMU_FLAG_CACHE_UC    (3ull << MMU_FLAG_CACHE_SHIFT)

#endif
