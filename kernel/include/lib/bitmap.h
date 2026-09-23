#ifndef LIB_BITMAP_H
#define LIB_BITMAP_H

#include <stdint.h>
#include <stddef.h>
#include <lib/memory.h>

typedef struct bitmap_s
{
  uint32_t* array;
  size_t bit_count;
} bitmap_t;

static inline void bitmap_init(bitmap_t *bitmap){
  if (!bitmap)
    return;

  bitmap->array = NULL;
  bitmap->bit_count = 0;
}

static inline void bitmap_set_bit_count(bitmap_t *bitmap, size_t bit_count){
  if (!bitmap)
    return;

  bitmap->bit_count = bit_count;
}

static inline size_t bitmap_index_from_bit(size_t bit)
{
  return bit >> 5;
}

static inline size_t bitmap_offset_from_bit(size_t bit)
{
  return bit & 31;
}

static inline void bitmap_set_bit(bitmap_t *bitmap, size_t bit)
{
  bitmap->array[bit >> 5] |= (1u << (bit & 31));
}

static inline void bitmap_clear_bit(bitmap_t *bitmap, size_t bit)
{
  bitmap->array[bit >> 5] &= ~(1u << (bit & 31));
}

static inline int bitmap_test_bit(bitmap_t *bitmap, size_t bit)
{
  return (bitmap->array[bit >> 5] >> (bit & 31)) & 1u;
}

static inline void bitmap_clear(bitmap_t* bitmap){
  if (!bitmap || !bitmap->array || bitmap->bit_count == 0)
    return;

  size_t total_words = (bitmap->bit_count + 31) >> 5;
  memset(bitmap->array, 0, total_words * sizeof(uint32_t));
}

static inline void bitmap_place(bitmap_t *bitmap, void *placement){
  if (!bitmap)
    return;

  bitmap->array = (uint32_t*)placement;
}

static inline void* bitmap_array_end(bitmap_t *bitmap)
{
  if(!bitmap)
    return NULL;

  size_t total_words = (bitmap->bit_count + 31) >> 5;
  return bitmap->array + (total_words * sizeof(uint32_t));
}

static inline uint32_t bitmap_get_word(bitmap_t *bitmap, size_t word_index)
{
  return bitmap->array[word_index];
}

#endif
