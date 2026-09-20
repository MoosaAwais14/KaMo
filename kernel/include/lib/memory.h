#ifndef LIB_MEMORY_H
#define LIB_MEMORY_H

#include <stdint.h>
#include <stddef.h>

#include <attributes.h>

#define ALIGN_UP(num, alignment)    (((num) + (alignment - 1)) & ~(alignment - 1))
#define ALIGN_DOWN(num, alignment)  ((num) & ~(alignment - 1))

static __always_inline void *memcpy(void *restrict dest, const void *restrict src, size_t count)
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

static __always_inline uint8_t *memset(void *dest, uint8_t val, size_t count)
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

static __always_inline void *memmove(void *dest, const void *src, size_t n)
{
  if (dest == src || n == 0)
    return dest;

  uint8_t *d = (uint8_t *)dest;
  const uint8_t *s = (const uint8_t *)src;

  size_t words = n / sizeof(unsigned long);
  size_t bytes = n % sizeof(unsigned long);

  if (d < s)
  {
    unsigned long *dw = (unsigned long *)d;
    const unsigned long *sw = (const unsigned long *)s;

    for (size_t i = 0; i < words; i++) {
      dw[i] = sw[i];
    }

    d = (uint8_t *)(dw + words);
    s = (const uint8_t *)(sw + words);

    for (size_t i = 0; i < bytes; i++) {
      d[i] = s[i];
    }
  }
  else
  {
    d += n;
    s += n;

    for (size_t i = 0; i < bytes; i++) {
      d--;
      s--;
      *d = *s;
    }

    unsigned long *dw = (unsigned long *)d;
    const unsigned long *sw = (const unsigned long *)s;

    for (size_t i = 0; i < words; i++) {
      dw--;
      sw--;
      *dw = *sw;
    }
  }

  return dest;
}

static __always_inline int memcmp(const void *s1, const void *s2, size_t n)
{
  const uint8_t *a = (const uint8_t *)s1;
  const uint8_t *b = (const uint8_t *)s2;

  for (size_t i = 0; i < n; i++)
  {
    if (a[i] != b[i])
      return (int)a[i] - (int)b[i];
  }

  return 0;
}

#endif
