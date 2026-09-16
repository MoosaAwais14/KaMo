#ifndef CPU_CPU_H
#define CPU_CPU_H

#include <cpu.h>

#include <cpu/gdt.h>
#include <cpu/idt.h>

typedef struct arch_cpu_s {
  gdt_cpu_t gdt;
  idt_cpu_t idt;

  uintptr_t kernel_stack;
} arch_cpu_t;

typedef uint32_t (*arch_cpu_current_id_fn_t)(void);
typedef uint32_t (*arch_cpu_cpu_count_fn_t)(void);

extern int arch_percpu_early_init(cpu_t* cpu);
extern int arch_percpu_init(cpu_t* cpu);

extern void arch_cpu_set_current_id_provider(arch_cpu_current_id_fn_t fn);
extern uint32_t arch_cpu_current_id(void);

extern void arch_cpu_set_cpu_count_provider(arch_cpu_cpu_count_fn_t fn);
extern uint32_t arch_cpu_cpu_count(void);

#endif
