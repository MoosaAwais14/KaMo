#ifndef KAMO_X86_INCLUDE_CPU_H
#define KAMO_X86_INCLUDE_CPU_H

#include <stdint.h>

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

#endif
