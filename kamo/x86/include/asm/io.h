#ifndef KAMO_X86_INCLUDE_ASM_IO_H
#define KAMO_X86_INCLUDE_ASM_IO_H

#include <stdint.h>
#include <stddef.h>

#include <attributes.h>

static __always_inline uint8_t x86_inb(uint16_t port)
{
  uint8_t rv;
  __asm__ volatile("inb %1, %0" : "=a"(rv) : "dN"(port));
  return rv;
}

static __always_inline void x86_outb(uint16_t port, uint8_t data)
{
  __asm__ volatile("outb %1, %0" ::"dN"(port), "a"(data));
}

#endif
