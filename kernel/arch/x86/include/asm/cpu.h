#ifndef ASM_CPU_H
#define ASM_CPU_H

#include <attributes.h>

static __always_inline void cpu_relax(void)
{
  __asm__ volatile("pause");
}

#endif
