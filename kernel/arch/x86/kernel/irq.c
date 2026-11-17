#include <asm/irq.h>

#include <kernel/irq.h>
#include <kernel/interrupt.h>

#include <asm/irqchip.h>
#include <asm/i8259.h>
#include <asm/idt.h>

#include <stddef.h>

int arch_irq_init(void)
{
  int ret = legacy_pic.init(NULL);
  if (ret != 0)
    return ret;

  for(size_t i = 0; i < 16; i++)
  {
    uint32_t vector = IDT_IRQ_VECTOR_BASE + i;

    if (irq_set_chip(i, &legacy_pic) != 0 ||
        irq_set_vector(i, vector) != 0 ||
        irq_register(i, irqchip_handle) != 0 ||
        irq_set_flags(i, IRQ_TRIGGER_DEFAULT | IRQ_POLARITY_DEFAULT) != 0 ||
        interrupt_set_irq(vector, i) != 0 ||
        irq_disable(i) != 0)
      return -1;
  }

  return 0;
}
