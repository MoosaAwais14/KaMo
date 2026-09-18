#ifndef KERNEL_INCLUDE_IRQ_H
#define KERNEL_INCLUDE_IRQ_H

#include <stdint.h>

#define IRQ_MAX 256

#define IRQ_VECTOR_INVALID ((uint32_t)-1)

struct irq_desc_s;

typedef enum irq_flags_e {
  IRQ_TRIGGER_DEFAULT = 0x00,
  IRQ_TRIGGER_EDGE    = 0x01,
  IRQ_TRIGGER_LEVEL   = 0x02,

  IRQ_POLARITY_DEFAULT = 0x00,
  IRQ_POLARITY_HIGH    = 0x04,
  IRQ_POLARITY_LOW     = 0x08,
} irq_flags_t;

typedef struct irq_chip_s {
  const char *name;

  int  (*init)(void *arg);
  void (*shutdown)(void);

  int  (*configure)(uint32_t irq, irq_flags_t flags);

  int  (*mask)(uint32_t irq);
  int  (*unmask)(uint32_t irq);

  void (*eoi)(uint32_t irq);
} irq_chip_t;

typedef void (*irq_handler_t)(struct irq_desc_s* desc);
typedef void (*irq_action_t)(uint32_t irq, void *arg);

typedef struct irq_desc_s {
  uint32_t irq;
  uint32_t vector;

  const irq_chip_t* chip;
  irq_flags_t flags;

  irq_handler_t handler;
  irq_action_t action;

  void *arg;

  uint8_t enabled;
} irq_desc_t;

extern int irq_init(void);

extern int irq_set_vector(uint32_t irq, uint32_t vector);
extern int irq_set_chip(uint32_t irq, const irq_chip_t* chip);
extern int irq_set_flags(uint32_t irq, irq_flags_t flags);

extern int irq_register(uint32_t irq, irq_handler_t handler);
extern int irq_unregister(uint32_t irq);

extern int irq_register_action(uint32_t irq, irq_action_t action, void* arg);
extern int irq_unregister_action(uint32_t irq);

extern int irq_enable(uint32_t irq);
extern int irq_disable(uint32_t irq);

extern void irq_dispatch(uint32_t irq);

extern uint32_t irq_get_vector(uint32_t irq);

#endif
