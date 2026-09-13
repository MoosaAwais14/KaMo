#ifndef ASM_IO_H
#define ASM_IO_H

#include <stdint.h>

#include <attributes.h>

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

static __always_inline void writeb(uintptr_t addr, uint8_t val) {
  *(volatile uint8_t*)addr = val;
}

static __always_inline uint8_t readb(uintptr_t addr) {
  return *(volatile uint8_t*)addr;
}

static __always_inline void writew(uintptr_t addr, uint16_t val) {
  *(volatile uint16_t*)addr = val;
}

static __always_inline uint16_t readw(uintptr_t addr) {
  return *(volatile uint16_t*)addr;
}

static __always_inline void writel(uintptr_t addr, uint32_t val) {
  *(volatile uint32_t*)addr = val;
}

static __always_inline uint32_t readl(uintptr_t addr) {
  return *(volatile uint32_t*)addr;
}

static __always_inline void writeq(uintptr_t addr, uint64_t val) {
  *(volatile uint64_t*)addr = val;
}

static __always_inline uint64_t readq(uintptr_t addr) {
  return *(volatile uint64_t*)addr;
}

#endif
