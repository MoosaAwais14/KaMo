#ifndef KAMO_X86_INCLUDE_CPU_H
#define KAMO_X86_INCLUDE_CPU_H

#include <stdint.h>

#include <cpuid.h>

static inline void x86_cpu_cli(void)
{
  __asm__ volatile("cli");
}

static inline void x86_cpu_sti(void)
{
  __asm__ volatile("sti");
}

static inline void x86_cpu_pause(void)
{
  __asm__ volatile("pause");
}

static inline void x86_cpu_halt(void)
{
  __asm__ volatile("hlt");
}

static inline void x86_cpu_write_cr3(uint32_t cr3)
{
  __asm__ volatile("mov %0, %%cr3" ::"r"(cr3));
}

static inline uint32_t x86_cpu_read_cr0(void)
{
  uint32_t val;
  __asm__ volatile ("mov %%cr0, %0" : "=r"(val));
  return val;
}

static inline void x86_cpu_write_cr0(uint32_t cr0)
{
  __asm__ volatile("mov %0, %%cr0" ::"r"(cr0));
}

static inline uint32_t x86_cpu_read_cr4(void)
{
  uint32_t val;
  __asm__ volatile ("mov %%cr4, %0" : "=r"(val));
  return val;
}

static inline void x86_cpu_write_cr4(uint32_t cr4)
{
  __asm__ volatile("mov %0, %%cr4" ::"r"(cr4));
}

static inline void x86_cpu_cpuid(unsigned int* eax, unsigned int* ebx, unsigned int* ecx, unsigned int* edx)
{
  __get_cpuid(1, eax, ebx, ecx, edx);
}

static inline uint8_t x86_cpu_pse_support(void)
{
  unsigned int eax, ebx, ecx, edx;
  x86_cpu_cpuid(&eax, &ebx, &ecx, &edx);
  return (edx & (1u << 3u));
}

#endif
