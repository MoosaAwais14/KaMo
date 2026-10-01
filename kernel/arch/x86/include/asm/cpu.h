#ifndef ASM_CPU_H
#define ASM_CPU_H

#if !defined(__ASSEMBLER__) && !defined(LINKER_SCRIPT)

#include <stdint.h>
#include <compiler/attributes.h>
#include <kernel/address.h>
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

static __always_inline void load_cr3(phys_addr_t cr3)
{
  uint32_t value = (uint32_t)cr3;
  __asm__ volatile("mov %0, %%cr3" :: "r"(value) : "memory");
}

static __always_inline phys_addr_t read_cr3(void)
{
  uint32_t value;
  __asm__ volatile("mov %%cr3, %0" : "=r"(value) :: "memory");
  return value;
}

#endif

#endif
