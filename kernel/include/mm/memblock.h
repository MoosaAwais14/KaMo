#ifndef MM_MEMBLOCK_H
#define MM_MEMBLOCK_H

#include <stdint.h>
#include <stddef.h>

#include <kernel/range.h>

typedef enum memblock_err_e {
  MEMBLOCK_OK,
  MEMBLOCK_ERR_INVALID,
  MEMBLOCK_ERR_EARLYBUMP,
  MEMBLOCK_ERR_NOMEM
} memblock_err_t;

typedef struct {
  void* node;
} memblock_iter_t;

extern memblock_err_t memblock_init(void);

extern memblock_err_t memblock_add(range_t physical_range);
extern memblock_err_t memblock_reserve(range_t physical_range);

extern memblock_err_t memblock_alloc(size_t size, size_t aligned, range_t* out);
extern memblock_err_t memblock_alloc_range(size_t size, size_t aligned, range_t range, range_t* out);
extern memblock_err_t memblock_alloc_free(range_t range);

extern memblock_err_t memblock_memory_first(memblock_iter_t *iter, range_t *out);
extern memblock_err_t memblock_memory_next(memblock_iter_t *iter, range_t *out);

extern memblock_err_t memblock_reserved_first(memblock_iter_t *iter, range_t *out);
extern memblock_err_t memblock_reserved_next(memblock_iter_t *iter, range_t *out);


#endif
