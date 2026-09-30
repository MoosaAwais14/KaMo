#ifndef ASM_IRQCHIP_H
#define ASM_IRQCHIP_H

#if !defined(__ASSEMBLER__) && !defined(LINKER_SCRIPT)

#include <kernel/irq.h>

extern void irqchip_handle(irq_desc_t* desc);

#endif

#endif
