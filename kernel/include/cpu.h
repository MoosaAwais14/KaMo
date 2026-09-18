#ifndef KERNEL_INCLUDE_CPU_H
#define KERNEL_INCLUDE_CPU_H

#include <stdint.h>

#define CPU_MAX 64 

typedef struct cpu_s {
  uint32_t id;
  uint8_t online;

  void* arch_priv;
} cpu_t;

extern cpu_t* cpu_current(void);
extern cpu_t* cpu_get(uint32_t id);

extern int cpu_early_init(uint32_t id);
extern int cpu_init(uint32_t id);

#endif
