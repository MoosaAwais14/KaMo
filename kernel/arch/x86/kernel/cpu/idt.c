#include <cpu/idt.h>

#include <stddef.h>

#include <cpu/gdt.h>

#include <lib/memory.h>

static inline uint8_t idt_default_flags(size_t vector);

static void idt_cpu_init_defaults(idt_cpu_t *idt);

extern void* idt_default_stubs[IDT_ENTRY_COUNT];

int idt_cpu_init(idt_cpu_t* idt_cpu)
{
  if (!idt_cpu)
    return -1;

  memset(idt_cpu, 0, sizeof(*idt_cpu));

  idt_cpu->ptr.base = (uint32_t)idt_cpu->entries;
  idt_cpu->ptr.limit = sizeof(idt_cpu->entries) - 1;

  idt_cpu_init_defaults(idt_cpu);

  return 0;
}

int idt_cpu_load(idt_cpu_t* idt_cpu)
{
  if(!idt_cpu)
    return -1;

  if(idt_cpu->ptr.base == 0 || idt_cpu->ptr.limit == 0)
    return 1;

  idt_load(&idt_cpu->ptr);

  return 0;
}


int idt_cpu_set_gate(idt_cpu_t* idt_cpu, uint8_t vector, uint32_t base, uint16_t selector, uint8_t flags)
{
  if (!idt_cpu)
    return -1;

  idt_set_gate(&idt_cpu->entries[vector], base, selector, flags);

  return 0;
}

void idt_dispatch(idt_frame_t* frame)
{
  // Do stuff here...
}

static void idt_cpu_init_defaults(idt_cpu_t* idt_cpu)
{
  for (size_t vector = 0; vector < IDT_ENTRY_COUNT; vector++) {
    void* stubs = idt_default_stubs[vector];
    uint8_t flags = idt_default_flags(vector);

    idt_cpu_set_gate(idt_cpu, vector, (uint32_t)stubs, GDT_KERNEL_CODE32_SELECTOR, flags);
  }
}

static inline uint8_t idt_default_flags(size_t vector)
{
  if (vector == 3 || vector == 4)
    return IDT_USER_TRAP_GATE;

  if (vector < IDT_IRQ_VECTOR_BASE)
    return IDT_DEFAULT_TRAP_GATE;

  return IDT_DEFAULT_INTERRUPT_GATE;
}
