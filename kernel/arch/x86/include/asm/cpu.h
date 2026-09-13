#ifndef ASM_CPU_H
#define ASM_CPU_H

#include <attributes.h>

static __always_inline void cpu_relax(void)
{
  __asm__ volatile("pause" ::: "memory");
}

static __always_inline void local_halt(void)
{
  __asm__ volatile("hlt" ::: "memory");
}

#endif
