#ifndef KERNEL_CONSOLE_H
#define KERNEL_CONSOLE_H

#include <stdint.h>
#include <stddef.h>

typedef struct console_s {
  const char* name;
} console_t;

#endif
