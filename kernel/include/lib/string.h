#ifndef KAMO_X86_INCLUDE_LIB_STRING_H
#define KAMO_X86_INCLUDE_LIB_STRING_H

#include "memory.h"

#include <attributes.h>

static __always_inline char to_lower_ascii(char c)
{
  if (c >= 'A' && c <= 'Z')
    return c + 32;
  return c;
}

static __always_inline size_t strlen(const char *s) 
{
  size_t len = 0;
  while (s[len])
    len++;
  return len;
}

static __always_inline int strcmp(const char *a, const char *b) 
{
  while (*a && (*a == *b)) {
    a++;
    b++;
  }
  return (unsigned char)*a - (unsigned char)*b;
}

static __always_inline int strncmp(const char *a, const char *b, size_t n) 
{
  while (n && *a && (*a == *b)) {
    a++;
    b++;
    n--;
  }

  if (n == 0)
    return 0;

  return (unsigned char)*a - (unsigned char)*b;
}

static __always_inline char *strcpy(char *dst, const char *src) 
{
  char *ret = dst;

  while ((*dst++ = *src++))
    ;

  return ret;
}

static __always_inline char *strncpy(char *dst, const char *src, size_t n) 
{
  char *ret = dst;

  while (n && *src) {
    *dst++ = *src++;
    n--;
  }

  while (n--) {
    *dst++ = '\0';
  }

  return ret;
}

static __always_inline char *strcat(char *dst, const char *src) 
{
  char *ret = dst;

  while (*dst)
    dst++;

  while ((*dst++ = *src++))
    ;

  return ret;
}

static __always_inline char *strncat(char *dst, const char *src, size_t n) 
{
  char *ret = dst;

  while (*dst)
    dst++;

  while (n && *src) {
    *dst++ = *src++;
    n--;
  }

  *dst = '\0';

  return ret;
}

static __always_inline char *strchr(const char *s, int c) 
{
  while (*s) {
    if (*s == (char)c)
      return (char *)s;
    s++;
  }

  return (c == 0) ? (char *)s : NULL;
}

static __always_inline char *strrchr(const char *s, int c) 
{
  const char *last = NULL;

  do {
    if (*s == (char)c)
      last = s;
  } while (*s++);

  return (char *)last;
}

static __always_inline int strcasecmp(const char *a, const char *b)
{
  while (*a && *b)
  {
    char ca = to_lower_ascii(*a);
    char cb = to_lower_ascii(*b);

    if (ca != cb)
      return (unsigned char)ca - (unsigned char)cb;

    a++;
    b++;
  }

  return (unsigned char)to_lower_ascii(*a) -
  (unsigned char)to_lower_ascii(*b);
}

static __always_inline int strncasecmp(const char *a, const char *b, size_t n)
{
  while (n && *a && *b)
  {
    char ca = to_lower_ascii(*a);
    char cb = to_lower_ascii(*b);

    if (ca != cb)
      return (unsigned char)ca - (unsigned char)cb;

    a++;
    b++;
    n--;
  }

  if (n == 0)
    return 0;

  return (unsigned char)to_lower_ascii(*a) -
  (unsigned char)to_lower_ascii(*b);
}

#endif
