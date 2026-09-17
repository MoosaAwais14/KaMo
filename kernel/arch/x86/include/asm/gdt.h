#ifndef ASM_GDT_H
#define ASM_GDT_H

#include <attributes.h>

#define GDT_SELECTOR(index, rpl)  \
  ((index << 3) | (rpl & 0x3))

#ifndef __ASSEMBLER__

#include <stdint.h>

typedef struct gdt_entry_s {
  uint16_t limit_low;
  uint16_t base_low;
  uint8_t  base_middle;
  uint8_t  access;
  uint8_t  granularity;
  uint8_t  base_high;
} __packed gdt_entry_t;

typedef struct gdt_ptr_s {
  uint16_t limit;
  uint32_t base;
} __packed gdt_ptr_t;

static inline void gdt_set_gate(gdt_entry_t* entry, uint32_t base, uint32_t limit, uint8_t access, uint8_t granularity)
{
  entry->base_low = (base & 0xFFFF);
  entry->base_middle = (base >> 16) & 0xFF;
  entry->base_high = (base >> 24) & 0xFF;

  entry->limit_low = (limit & 0xFFFF);
  entry->granularity = ((limit >> 16) & 0x0F);

  entry->granularity |= (granularity & 0xF0);
  entry->access = access;
}

static inline void gdt_load(const void *gdt_ptr, uint16_t code_sel, uint16_t data_sel) 
{
  __asm__ volatile (
    "lgdt (%0)\n\t"

    "mov %1, %%ax\n\t"
    "mov %%ax, %%ds\n\t"
    "mov %%ax, %%es\n\t"
    "mov %%ax, %%fs\n\t"
    "mov %%ax, %%gs\n\t"
    "mov %%ax, %%ss\n\t"

    "push %2\n\t"
    "push $1f\n\t"
    "lretl\n\t"

    "1:\n\t"

    :
    : "r"(gdt_ptr), "r"(data_sel), "r"(code_sel)
    : "ax", "memory"
  );
}

static inline void gdt_reload_segments(uint16_t code_sel, uint16_t data_sel) 
{
  __asm__ volatile (
    "mov %0, %%ax\n\t"
    "mov %%ax, %%ds\n\t"
    "mov %%ax, %%es\n\t"
    "mov %%ax, %%fs\n\t"
    "mov %%ax, %%gs\n\t"
    "mov %%ax, %%ss\n\t"

    "push %1\n\t"
    "push $1f\n\t"
    "lretl\n\t"

    "1:\n\t"

    :
    : "r"(data_sel), "r"(code_sel)
    : "ax", "memory"
  );
}

#endif

#endif
