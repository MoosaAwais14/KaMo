#ifndef CPU_GDT_H
#define CPU_GDT_H

#include <asm/gdt.h>

#include <asm/tss.h>

#include <attributes.h>

#define GDT_NULL_INDEX          0x00
#define GDT_KERNEL_CODE32_INDEX 0x01
#define GDT_KERNEL_DATA32_INDEX 0x02
#define GDT_USER_CODE32_INDEX   0x03
#define GDT_USER_DATA32_INDEX   0x04
#define GDT_TSS_INDEX           0x05

#define GDT_ENTRY_COUNT         7

#ifndef __ASSEMBLER__

typedef struct gdt_cpu_s {
  gdt_entry_t entries[GDT_ENTRY_COUNT];
  gdt_ptr_t ptr;

  tss_entry_t tss_entry;
} __aligned(16) gdt_cpu_t;

extern int gdt_cpu_init(gdt_cpu_t* gdt_cpu);
extern int gdt_cpu_load_kernel(const gdt_cpu_t* gdt_cpu);
extern void gdt_cpu_reload_kernel_segments(void);

extern int gdt_cpu_tss_init(gdt_cpu_t* gdt_cpu);
extern int gdt_cpu_tss_load(gdt_cpu_t* gdt_cpu);
extern int gdt_cpu_tss_set_kernel_stack(gdt_cpu_t* gdt_cpu, uintptr_t stack);

#endif

#define GDT_KERNEL_CODE32_SELECTOR  \
    GDT_SELECTOR(GDT_KERNEL_CODE32_INDEX, 0)

#define GDT_KERNEL_DATA32_SELECTOR  \
    GDT_SELECTOR(GDT_KERNEL_DATA32_INDEX, 0)

#define GDT_USER_CODE32_SELECTOR  \
    GDT_SELECTOR(GDT_USER_CODE32_INDEX, 3)

#define GDT_USER_DATA32_SELECTOR  \
    GDT_SELECTOR(GDT_USER_DATA32_INDEX, 3)

#define GDT_TSS_SELECTOR  \
    GDT_SELECTOR(GDT_TSS_INDEX, 0)

#endif
