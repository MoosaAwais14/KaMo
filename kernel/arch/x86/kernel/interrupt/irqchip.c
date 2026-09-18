#include <interrupt/irqchip.h>

void irqchip_handle(irq_desc_t* desc)
{
  const irq_chip_t* chip = desc->chip;

  if (chip->mask)
    chip->mask(desc->irq);

  if (desc->action)
    desc->action(desc->irq, desc->arg);

  if (chip->eoi)
    chip->eoi(desc->irq);

  if (chip->unmask)
    chip->unmask(desc->irq);
}
