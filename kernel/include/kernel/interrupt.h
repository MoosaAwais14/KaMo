#ifndef KERNEL_INTERRUPT_H
#define KERNEL_INTERRUPT_H

#include <stdint.h>

#include <sync/spinlock.h>

#define INTERRUPT_VECTOR_COUNT 256

typedef enum interrupt_type {
  INTERRUPT_NONE = 0,
  INTERRUPT_IRQ,
  INTERRUPT_TRAP,
  INTERRUPT_NMI,
  INTERRUPT_SOFTWARE,
} interrupt_type_t;

typedef struct interrupt_context_s {
  uint32_t vector;
  void* arch;
} interrupt_context_t;

typedef struct interrupt_desc_s {
    uint32_t vector;
    interrupt_type_t type;
    uint32_t irq;

    raw_spinlock_t rlock;
} interrupt_desc_t;

extern int interrupt_init(void);

extern int interrupt_set_type(uint32_t vector, interrupt_type_t type);
extern int interrupt_set_irq(uint32_t vector, uint32_t irq);

extern void interrupt_dispatch(interrupt_context_t* context);

#endif
