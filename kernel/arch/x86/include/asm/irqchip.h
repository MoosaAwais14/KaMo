#ifndef ASM_IRQCHIP_H
#define ASM_IRQCHIP_H

#include <kernel/irq.h>

extern void irqchip_handle(irq_desc_t* desc);

#endif
