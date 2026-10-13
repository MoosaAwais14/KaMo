#ifndef MM_SLAB_H
#define MM_SLAB_H

#include <stdint.h>
#include <stddef.h>

typedef enum slab_index_e {
  SLAB_INDEX_16 = 0,
  SLAB_INDEX_32,
  SLAB_INDEX_64,
  SLAB_INDEX_128,
  SLAB_INDEX_256,
  SLAB_INDEX_512,
  SLAB_INDEX_1024,
  SLAB_INDEX_2048,
  SLAB_INDEX_4096,
  SLAB_INDEX_8192,
  SLAB_INDEX_COUNT
} slab_index_t;

extern void slab_init(void);

extern void* kmalloc(size_t size);
extern void kfree(void* ptr);

#endif
