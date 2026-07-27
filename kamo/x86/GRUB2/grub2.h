#ifndef GRUB2_H
#define GRUB2_H

#include <stdint.h>
#include <stddef.h>

#define INT32_TO_BYTES(x)        \
  (uint8_t)((x) & 0xFF),         \
  (uint8_t)(((x) >> 8) & 0xFF),  \
  (uint8_t)(((x) >> 16) & 0xFF), \
  (uint8_t)(((x) >> 24) & 0xFF)

#define INT16_TO_BYTES(x)        \
  (uint8_t)((x) & 0xFF),         \
  (uint8_t)(((x) >> 8) & 0xFF)   \

#define __boot __attribute__((section(".boot")))

#include "multiboot2.h"

#define MULTIBOOT2_HEADER(magic, arch, length)  \
  INT32_TO_BYTES(magic),                        \
  INT32_TO_BYTES(arch),                         \
  INT32_TO_BYTES(length),                       \
  INT32_TO_BYTES(-(magic + arch + length))

#define MULTIBOOT2_HEADER_TAG(type, flags, size, ...) \
  INT16_TO_BYTES(type),                             \
  INT16_TO_BYTES(flags),                            \
  INT32_TO_BYTES(size),                             \
  __VA_ARGS__

#endif
