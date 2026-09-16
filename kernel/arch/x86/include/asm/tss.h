#ifndef ASM_TSS_H
#define ASM_TSS_H

#include <attributes.h>

#ifndef __ASSEMBLER__

#include <stdint.h>

typedef struct tss_entry_s {
    uint32_t prev_tss;

    uint32_t esp0;
    uint32_t ss0;

    uint32_t esp1;
    uint32_t ss1;

    uint32_t esp2;
    uint32_t ss2;

    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;

    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;

    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;

    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;

    uint32_t ldt;

    uint16_t trap;
    uint16_t iomap_base;
} __packed tss_entry_t;

static inline void tss_load(uint16_t gdt_tss_index){
  __asm__ volatile (
    "mov %0, %%ax\n\t"
    "ltr %%ax"
    :
    : "r"(gdt_tss_index)
    : "ax", "memory"
  );
}

#endif

#endif
