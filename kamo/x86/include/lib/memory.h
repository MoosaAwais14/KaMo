#ifndef KAMO_X86_INCLUDE_LIB_MEMORY_H
#define KAMO_X86_INCLUDE_LIB_MEMORY_H

#include <stdint.h>
#include <stddef.h>

#define ALIGN_UP(num, alignment)    (((num) + (alignment - 1)) & ~(alignment - 1))
#define ALIGN_DOWN(num, alignment)  ((num) & ~(alignment - 1))

static inline void *memcpy(void *restrict dest, const void *restrict src, size_t count)
{
  unsigned long *d_word = (unsigned long *)dest;
  const unsigned long *s_word = (const unsigned long *)src;

  size_t words = count / sizeof(unsigned long);
  size_t bytes = count % sizeof(unsigned long);

  for (size_t i = 0; i < words; i++) {
    d_word[i] = s_word[i];
  }

  uint8_t *d_byte = (uint8_t *)(d_word + words);
  const uint8_t *s_byte = (const uint8_t *)(s_word + words);

  for (size_t i = 0; i < bytes; i++) {
    d_byte[i] = s_byte[i];
  }

  return dest;
}

static inline uint8_t *memset(void *dest, uint8_t val, size_t count)
{
  uint8_t *d = (uint8_t *)dest;

  unsigned long word_val = val;
  for (size_t i = 1; i < sizeof(unsigned long); i++) {
    word_val |= ((unsigned long)val) << (i * 8);
  }

  size_t words = count / sizeof(unsigned long);
  size_t bytes = count % sizeof(unsigned long);

  unsigned long *d_word = (unsigned long *)d;
  for (size_t i = 0; i < words; i++) {
    d_word[i] = word_val;
  }

  d = (uint8_t *)(d_word + words);
  for (size_t i = 0; i < bytes; i++) {
    d[i] = val;
  }

  return (uint8_t *)dest;
}

#endif
