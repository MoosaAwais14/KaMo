#include <cpu/gdt.h>

#include <lib/memory.h>

int gdt_cpu_init(gdt_cpu_t* gdt_cpu)
{
  if(!gdt_cpu)
    return - 1;

  gdt_cpu->ptr.base = (uint32_t)gdt_cpu->entries;
  gdt_cpu->ptr.limit = sizeof(gdt_cpu->entries) - 1;

  gdt_set_gate(&gdt_cpu->entries[GDT_NULL_INDEX], 0x00000000, 0x00000000, 0x00, 0x00);
  
  gdt_set_gate(&gdt_cpu->entries[GDT_KERNEL_CODE32_INDEX], 0x00000000, 0xFFFFFFFF, 0x9A, 0xCF);
  gdt_set_gate(&gdt_cpu->entries[GDT_KERNEL_DATA32_INDEX], 0x00000000, 0xFFFFFFFF, 0x92, 0xCF);

  gdt_set_gate(&gdt_cpu->entries[GDT_USER_CODE32_INDEX], 0x00000000, 0xFFFFFFFF, 0xFA, 0xCF);
  gdt_set_gate(&gdt_cpu->entries[GDT_USER_DATA32_INDEX], 0x00000000, 0xFFFFFFFF, 0xF2, 0xCF);

  return 0;
}

int gdt_cpu_load_kernel(const gdt_cpu_t* gdt_cpu)
{
  if(!gdt_cpu)
    return - 1;

  if(gdt_cpu->ptr.base == 0 || gdt_cpu->ptr.limit == 0)
    return 1;

  gdt_load(&gdt_cpu->ptr, GDT_KERNEL_CODE32_SELECTOR, GDT_KERNEL_DATA32_SELECTOR);

  return 0;
}

void gdt_cpu_reload_kernel_segments(void)
{
  gdt_reload_segments(GDT_KERNEL_CODE32_SELECTOR, GDT_KERNEL_DATA32_SELECTOR);
}

int gdt_cpu_tss_init(gdt_cpu_t* gdt_cpu)
{
  if(!gdt_cpu)
    return - 1;

  gdt_cpu->tss_entry.ss0 = GDT_KERNEL_DATA32_SELECTOR;
  gdt_cpu->tss_entry.esp0 = 0x00000000;
  gdt_cpu->tss_entry.iomap_base = sizeof(tss_entry_t);

  uint32_t base = (uint32_t)&gdt_cpu->tss_entry;
  uint32_t limit = sizeof(tss_entry_t) - 1;

  gdt_set_gate(&gdt_cpu->entries[GDT_TSS_INDEX], base, limit, 0x89, 0x40);

  return 0;
}

int gdt_cpu_tss_load(gdt_cpu_t* gdt_cpu)
{
  if(!gdt_cpu)
    return - 1;
 
  if(gdt_cpu->tss_entry.ss0 == 0 || gdt_cpu->tss_entry.iomap_base == 0)
    return 1;

  tss_load(GDT_TSS_SELECTOR);

  return 0;
}

int gdt_cpu_tss_set_kernel_stack(gdt_cpu_t* gdt_cpu, uintptr_t stack)
{
  if(!gdt_cpu)
    return - 1;

  uint32_t esp0 = (uint32_t)stack;

  gdt_cpu->tss_entry.ss0 = GDT_KERNEL_DATA32_SELECTOR;
  gdt_cpu->tss_entry.esp0 = esp0;
  gdt_cpu->tss_entry.iomap_base = sizeof(tss_entry_t);

  return 0;
}
