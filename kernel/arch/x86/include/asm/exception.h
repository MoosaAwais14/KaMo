#ifndef ASM_EXCEPTION_H
#define ASM_EXCEPTION_H

#include <kernel/interrupt.h>

#include <sync/spinlock.h>

#include <asm/idt_arch.h>

typedef void (*exception_handler_t)(idt_frame_t* idt);

typedef struct exception_desc_s {
  exception_handler_t handler;
  uint32_t flags;

  raw_spinlock_t rlock;
} exception_desc_t;

#define ARCH_EXCEPTION_COUNT 64

extern void arch_exception_early_init(void);

extern void arch_exception_dispatch(interrupt_context_t* context);

#endif
