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

static inline slab_index_t get_slab_index(size_t size)
{
  if (size <= 16)   return SLAB_INDEX_16;
  if (size <= 32)   return SLAB_INDEX_32;
  if (size <= 64)   return SLAB_INDEX_64;
  if (size <= 128)  return SLAB_INDEX_128;
  if (size <= 256)  return SLAB_INDEX_256;
  if (size <= 512)  return SLAB_INDEX_512;
  if (size <= 1024) return SLAB_INDEX_1024;
  if (size <= 2048) return SLAB_INDEX_2048;
  if (size <= 4096) return SLAB_INDEX_4096;
  if (size <= 8192) return SLAB_INDEX_8192;
  return SLAB_INDEX_COUNT;
}

static inline size_t get_slab_size(slab_index_t slab_index)
{
  return (16UL << slab_index);
}

#endif
