#ifndef KERNEL_CPU_H
#define KERNEL_CPU_H

#include <stdint.h>
#include <stddef.h>

#include <mm/mm.h>

#include <compiler/attributes.h>

#define CPU_MAX 64 
typedef struct cpu_s {
  uint32_t id;
  uint8_t online;
  size_t preempt_count;
  mm_space_t* active_mm;
  void* arch_priv;
} __aligned(64) cpu_t;

extern cpu_t* cpu_current(void);
extern cpu_t* cpu_get(uint32_t id);

extern int cpu_early_init(uint32_t id);
extern int cpu_init(uint32_t id, mm_space_t* mm);

// TODO: place preempt stuff in a different file
//

static inline void preempt_disable(void)
{
  cpu_t* cpu = cpu_current();
  if (cpu) {
    cpu->preempt_count++;
  }
}

static inline void preempt_enable(void)
{
  cpu_t* cpu = cpu_current();
  if (cpu) {
    cpu->preempt_count--;
  }
}

#endif
