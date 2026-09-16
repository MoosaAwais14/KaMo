#ifndef CPU_IDT_H
#define CPU_IDT_H

#include <asm/idt.h>

#include <attributes.h>

#define IDT_ENTRY_COUNT 256

#ifndef __ASSEMBLER__

typedef struct idt_frame_s {
  uint32_t edi;
  uint32_t esi;
  uint32_t ebp;
  uint32_t esp;
  uint32_t ebx;
  uint32_t edx;
  uint32_t ecx;
  uint32_t eax;

  uint32_t ds;

  uint32_t vector;
  uint32_t error_code;

  uint32_t eip;
  uint32_t cs;
  uint32_t eflags;
  uint32_t useresp;
  uint32_t ss;
} idt_frame_t;

typedef struct idt_cpu_s {
  idt_entry_t entries[IDT_ENTRY_COUNT];
  idt_ptr_t ptr;
} __aligned(16) idt_cpu_t;

typedef void (*idt_interrupt_dispatch_fn_t)(void);

extern int idt_cpu_init(idt_cpu_t* idt_cpu);
extern int idt_cpu_load(idt_cpu_t* idt_cpu);

extern int idt_cpu_set_gate(idt_cpu_t* idt_cpu, uint8_t vector, uint32_t base, uint16_t selector, uint8_t flags);

#endif

#endif
