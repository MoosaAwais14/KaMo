#ifndef GRUB2_H
#define GRUB2_H

#include <stdint.h>
#include <stddef.h>

#define __boot __attribute__((section(".boot")))

#include "platform.h"

#include "multiboot2.h"

#define GRUB2_MULTIBOOT_HEADER(arch, length)                                    \
  (struct multiboot_header){                                                    \
    .magic = (uint32_t)MULTIBOOT2_HEADER_MAGIC,                                 \
    .architecture = (uint32_t)arch,                                             \
    .header_length = (uint32_t)length,                                          \
    .checksum = (uint32_t)(-(uint32_t)(MULTIBOOT2_HEADER_MAGIC + arch + length))\
  }

#define GRUB2_MULTIBOOT_HEADER_TAG(type, flags, size, ...)\
  INT16_TO_LE_BYTES(type),                                \
  INT16_TO_LE_BYTES(flags),                               \
  INT32_TO_LE_BYTES(size),                                \
  __VA_ARGS__

typedef struct grub2_multiboot_header_s 
{
  struct multiboot_header header;
  uint8_t tags_raw[];
} __attribute__ ((__packed__)) grub2_multiboot_header_t;

#endif
