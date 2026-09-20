#include <asm/irq.h>

#include <kernel/irq.h>
#include <kernel/interrupt.h>

#include <asm/irqchip.h>
#include <asm/i8259.h>
#include <asm/idt.h>

#include <stddef.h>

int arch_irq_init(void)
{
  legacy_pic.init(NULL);

  for(size_t i = 0; i < 16; i++)
  {
    irq_disable(i);
    irq_set_chip(i, &legacy_pic);
    irq_register(i, irqchip_handle);
    irq_set_flags(i, IRQ_TRIGGER_DEFAULT | IRQ_POLARITY_DEFAULT);
    interrupt_set_irq(IDT_IRQ_VECTOR_BASE + i, i);
  }

  return 0;
}
