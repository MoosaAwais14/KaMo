#ifndef ASM_CPU_H
#define ASM_CPU_H

#if !defined(__ASSEMBLER__) && !defined(LINKER_SCRIPT)

#include <stdint.h>
#include <compiler/attributes.h>
#include <asm/irqflags.h>

static __always_inline void cpu_relax(void)
{
  __asm__ volatile("pause" ::: "memory");
}

static __always_inline void local_halt(void)
{
  __asm__ volatile("hlt" ::: "memory");
}

static __always_inline __noreturn void local_safe_halt(void)
{
  local_irq_disable();
  
  for(;;)
  {
    local_halt();
  }
}

#endif

#endif
