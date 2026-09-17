#ifndef ASM_IDT_H
#define ASM_IDT_H

#include <attributes.h>

#define IDT_IRQ_VECTOR_BASE 0x20

#define IDT_DEFAULT_INTERRUPT_GATE 0x8E
#define IDT_DEFAULT_TRAP_GATE      0x8F
#define IDT_USER_TRAP_GATE         0xEF

#ifndef __ASSEMBLER__

#include <stdint.h>

typedef struct idt_entry_s {
  uint16_t base_low;
  uint16_t selector;
  uint8_t  always0;
  uint8_t  flags;
  uint16_t base_high;
} __packed idt_entry_t;

typedef struct idt_ptr_s {
  uint16_t limit;
  uint32_t base;
} __packed idt_ptr_t;

static inline void idt_set_gate(idt_entry_t* entry, uint32_t base, uint16_t selector, uint8_t flags){
  entry->base_high = (base >> 16) & 0xFFFF;
  entry->base_low = (base & 0xFFFF);
  entry->always0 = 0;
  entry->selector = selector;
  entry->flags = flags;
}

static inline void idt_load(const void* idt_ptr) {
  __asm__ volatile(
    "lidt (%0)"
    :
    : "r"(idt_ptr)
    : "ax", "memory"
  );
}

#endif

#endif
