#ifndef MM_MEMBLOCK_H
#define MM_MEMBLOCK_H

#include <stdint.h>
#include <stddef.h>

#include <kernel/range.h>

typedef enum memblock_err_e {
  MEMBLOCK_OK,
  MEMBLOCK_ERR_INVALID,
  MEMBLOCK_ERR_EARLYBUMP,
} memblock_err_t;

extern memblock_err_t memblock_init(void);
extern memblock_err_t memblock_add(range_t physical_range);
extern memblock_err_t memblock_reserve(range_t physical_range);
extern void* memblock_alloc(size_t size);
extern uintptr_t memblock_start(void);
extern uintptr_t memblock_end(void);

#endif
