#ifndef ASM_IO_H
#define ASM_IO_H

#if !defined(__ASSEMBLER__) && !defined(LINKER_SCRIPT)

#include <stdint.h>

#include <compiler/attributes.h>
#include <kernel/address.h>

static __always_inline uint8_t inb(uint16_t port)
{
  uint8_t rv;
  __asm__ volatile("inb %1, %0" : "=a"(rv) : "dN"(port) : "memory");
  return rv;
}

static __always_inline void outb(uint16_t port, uint8_t data)
{
  __asm__ volatile("outb %1, %0" ::"dN"(port), "a"(data) : "memory");
}

static __always_inline uint16_t inw(uint16_t port)
{
  uint16_t rv;
  __asm__ volatile("inw %1, %0" : "=a"(rv) : "dN"(port) : "memory");
  return rv;
}

static __always_inline void outw(uint16_t port, uint16_t data)
{
  __asm__ volatile("outw %1, %0" ::"dN"(port), "a"(data) : "memory");
}

static __always_inline uint32_t inl(uint16_t port) {
  uint32_t rv;
  __asm__ volatile ("inl %1, %0" : "=a"(rv) : "dN"(port) : "memory");
  return rv;
}

static __always_inline void outl(uint16_t port, uint32_t data) {
  __asm__ volatile ("outl %1, %0" :: "dN"(port), "a"(data) : "memory");
}

static __always_inline void writeb(virt_addr_t addr, uint8_t val) {
  *(volatile uint8_t*)addr = val;
}

static __always_inline uint8_t readb(virt_addr_t addr) {
  return *(volatile uint8_t*)addr;
}

static __always_inline void writew(virt_addr_t addr, uint16_t val) {
  *(volatile uint16_t*)addr = val;
}

static __always_inline uint16_t readw(virt_addr_t addr) {
  return *(volatile uint16_t*)addr;
}

static __always_inline void writel(virt_addr_t addr, uint32_t val) {
  *(volatile uint32_t*)addr = val;
}

static __always_inline uint32_t readl(virt_addr_t addr) {
  return *(volatile uint32_t*)addr;
}

static __always_inline void writeq(virt_addr_t addr, uint64_t val) {
  *(volatile uint64_t*)addr = val;
}

static __always_inline uint64_t readq(virt_addr_t addr) {
  return *(volatile uint64_t*)addr;
}

#endif

#endif
