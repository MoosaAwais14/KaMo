#ifndef LIB_BITMAP_H
#define LIB_BITMAP_H

#include <stdint.h>
#include <stddef.h>
#include <lib/memory.h>

#define BITMAP_WORD_TYPE    unsigned long
#define BITMAP_WORD_BITS    (sizeof(BITMAP_WORD_TYPE) * 8)

#define BITMAP_WORD_SHIFT         \
  ((BITMAP_WORD_BITS == 16) ? 4 : \
  (BITMAP_WORD_BITS == 32) ? 5 :  \
  (BITMAP_WORD_BITS == 64) ? 6 :  \
  0)

_Static_assert(
    BITMAP_WORD_BITS == 16 ||
    BITMAP_WORD_BITS == 32 ||
    BITMAP_WORD_BITS == 64,
    "Unsupported bitmap word size"
);

#define BITMAP_WORD_MASK    (BITMAP_WORD_BITS - 1)

#define BITMAP_WORD_COUNT(bits) \
    (((bits) + BITMAP_WORD_BITS - 1) / BITMAP_WORD_BITS)

#define BITMAP_SIZE(bits) \
    (BITMAP_WORD_COUNT(bits) * sizeof(BITMAP_WORD_TYPE))

#define BITMAP_WORD_INDEX(bit) \
    ((bit) >> BITMAP_WORD_SHIFT)

#define BITMAP_BIT_OFFSET(bit) \
    ((bit) & BITMAP_WORD_MASK)

typedef struct bitmap_s
{
    BITMAP_WORD_TYPE *array;
    size_t bit_count;
} bitmap_t;

static inline void bitmap_init(bitmap_t *bitmap)
{
    if (!bitmap)
        return;

    bitmap->array = NULL;
    bitmap->bit_count = 0;
}

static inline void bitmap_set_bit_count(bitmap_t *bitmap, size_t bit_count)
{
    if (!bitmap)
        return;

    bitmap->bit_count = bit_count;
}

static inline void bitmap_set_bit(bitmap_t *bitmap, size_t bit)
{
    bitmap->array[BITMAP_WORD_INDEX(bit)] |=
        (1ul << BITMAP_BIT_OFFSET(bit));
}

static inline void bitmap_clear_bit(bitmap_t *bitmap, size_t bit)
{
    bitmap->array[BITMAP_WORD_INDEX(bit)] &=
        ~(1ul << BITMAP_BIT_OFFSET(bit));
}

static inline int bitmap_test_bit(bitmap_t *bitmap, size_t bit)
{
    return (bitmap->array[BITMAP_WORD_INDEX(bit)] >>
            BITMAP_BIT_OFFSET(bit)) & 1ul;
}

static inline void bitmap_clear(bitmap_t *bitmap)
{
    if (!bitmap || !bitmap->array || bitmap->bit_count == 0)
        return;

    memset(
        bitmap->array,
        0,
        BITMAP_SIZE(bitmap->bit_count)
    );
}

static inline void bitmap_place(bitmap_t *bitmap, void *placement)
{
    if (!bitmap)
        return;

    bitmap->array = (BITMAP_WORD_TYPE *)placement;
}

static inline void *bitmap_array_end(bitmap_t *bitmap)
{
    if (!bitmap || !bitmap->array)
        return NULL;

    return bitmap->array + BITMAP_WORD_COUNT(bitmap->bit_count);
}

static inline BITMAP_WORD_TYPE bitmap_get_word(bitmap_t *bitmap, size_t word_index)
{
    return bitmap->array[word_index];
}

#endif

