#include <asm/irqchip.h>

void irqchip_handle(irq_desc_t* desc)
{
  if (!desc)
    return;

  unsigned long irq_flags = raw_spin_lock_irqsave(&desc->rlock);
  const irq_chip_t* chip = desc->chip;
  const uint32_t irq = desc->irq;
  irq_action_t action = desc->action;
  void* arg = desc->arg;
  raw_spin_unlock_irqrestore(&desc->rlock, irq_flags);

  if (!chip)
    return;

  if (chip->mask)
    chip->mask(irq);

  if (action)
    action(irq, arg);

  if (chip->eoi)
    chip->eoi(irq);

  irq_flags = raw_spin_lock_irqsave(&desc->rlock);
  if (desc->chip == chip && desc->enabled && chip->unmask)
    chip->unmask(irq);
  raw_spin_unlock_irqrestore(&desc->rlock, irq_flags);
}
