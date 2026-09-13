#ifndef GRUB2_H
#define GRUB2_H

#include <stdint.h>
#include <stddef.h>

typedef enum grub2_err_e {
  GRUB2_ERR_MMAP_MAXED = 3,
  GRUB2_ERR_CMDLINE_MAXED = 2,
  GRUB2_ERR_BAD_MAGIC = 1,

  GRUB2_OK = 0,

  GRUB2_ERR_GENERIC = -1,
  GRUB2_ERR_BAD_ARGS = -2,
  GRUB2_ERR_UNK = -3,
  GRUB2_ERR_NOT_INITIALIZED = -4,
  GRUB2_ERR_ALREADY_INITIALIZED = -5
} grub2_err_t;

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

/*
  * 
  * UART
  *
*/

extern grub2_err_t uart_init(uint16_t port_address, uint16_t baud_rate);
extern void uart_shutdown(void);
extern grub2_err_t uart_write_byte(int8_t data);

/*
  * 
  * MULTIBOOT2_TO_BOOT_INFO
  *
*/

#include <boot_info.h>

extern grub2_err_t multiboot2_to_boot_info(uint32_t magic, uintptr_t ptr, boot_info_t* boot_info);

/*
  * 
  * UART Implemention
  *
*/

static inline void putc(char c)
{
  if(c == '\n')
    uart_write_byte('\r');

  uart_write_byte(c);
}

static inline void puts(const char* str)
{
  while(*str)
  {
    putc(*str++);
  }
}

#endif
