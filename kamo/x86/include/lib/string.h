#ifndef KAMO_X86_INCLUDE_LIB_STRING_H
#define KAMO_X86_INCLUDE_LIB_STRING_H

#include "memory.h"

static inline size_t strlen(const char *s) {
  size_t len = 0;
  while (s[len])
    len++;
  return len;
}

static inline char *strcpy(char *dst, const char *src) {
  char *ret = dst;

  while ((*dst++ = *src++))
    ;

  return ret;
}



#endif
