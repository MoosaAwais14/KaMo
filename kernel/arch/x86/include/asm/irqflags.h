#ifndef ASM_IRQFLAGS_H
#define ASM_IRQFLAGS_H

#include <stdint.h>

#include <attributes.h>

static __always_inline void local_irq_disable(void)
{
  __asm__ volatile("cli" ::: "memory");
}

static __always_inline void local_irq_enable(void)
{
  __asm__ volatile("sti" ::: "memory");
}

static __always_inline uint32_t local_save_flags(void) {
  uint32_t flags;
  __asm__ volatile(
    "pushfl\n\t"
    "popl %0"
    : "=r"(flags)
    :
    : "memory"
  );
  return flags;
}

static __always_inline void local_restore_flags(uint32_t flags) {
  __asm__ volatile(
    "pushl %0\n\t"
    "popfl"
    :
    : "r"(flags)
    : "memory", "cc"
  );
}

static __always_inline uint32_t local_irq_save(void)
{
  uint32_t flags = local_save_flags();
  local_irq_disable();
  return flags;
}

#endif
