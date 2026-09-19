#include <irq.h>

#include <interrupt/irq.h>

#include <lib/memory.h>

static irq_desc_t irq_descs[IRQ_MAX] = { 0 };

int irq_init(void)
{
  memset(irq_descs, 0, sizeof(irq_descs));

  for (uint32_t irq = 0; irq < IRQ_MAX; irq++) {
    irq_desc_t* desc = &irq_descs[irq];

    desc->irq = irq;
    desc->vector = IRQ_VECTOR_INVALID;
    desc->chip = NULL;
    desc->rlock = RAW_SPINLOCK_UNLOCKED;
    desc->flags = IRQ_TRIGGER_DEFAULT | IRQ_POLARITY_DEFAULT;
    desc->enabled = 0;
  }

  return arch_irq_init();
}

int irq_set_vector(uint32_t irq, uint32_t vector)
{
  if(irq >= IRQ_MAX)
    return -1;

  irq_desc_t* desc = &irq_descs[irq];

  unsigned long flags = raw_spin_lock_irqsave(&desc->rlock);
  desc->vector = vector;
  raw_spin_unlock_irqrestore(&desc->rlock, flags);

  return 0;
}

int irq_set_chip(uint32_t irq, const irq_chip_t *chip)
{
  if(irq >= IRQ_MAX)
    return -1;

  irq_desc_t* desc = &irq_descs[irq];

  unsigned long flags = raw_spin_lock_irqsave(&desc->rlock);
  desc->chip = chip;
  raw_spin_unlock_irqrestore(&desc->rlock, flags);

  return 0;
}

int irq_set_flags(uint32_t irq, irq_flags_t flags)
{
  if(irq >= IRQ_MAX)
    return -1;

  irq_desc_t* desc = &irq_descs[irq];

  unsigned long f = raw_spin_lock_irqsave(&desc->rlock);

  desc->flags = flags;

  if (desc->chip && desc->chip->configure){
    int ret = desc->chip->configure(irq, flags);
    raw_spin_unlock_irqrestore(&desc->rlock, f);
    return ret;
  }

  raw_spin_unlock_irqrestore(&desc->rlock, f);
  return 0;
}

int irq_register(uint32_t irq, irq_handler_t handler)
{
  if(irq >= IRQ_MAX)
    return -1;

  irq_desc_t* desc = &irq_descs[irq];

  unsigned long flags = raw_spin_lock_irqsave(&desc->rlock);
  desc->handler = handler;
  raw_spin_unlock_irqrestore(&desc->rlock, flags);

  return 0;
}

int irq_unregister(uint32_t irq)
{
  if(irq >= IRQ_MAX)
    return -1;

  irq_desc_t* desc = &irq_descs[irq];

  unsigned long flags = raw_spin_lock_irqsave(&desc->rlock);
  desc->handler = NULL;
  raw_spin_unlock_irqrestore(&desc->rlock, flags);

  return 0;
}

int irq_register_action(uint32_t irq, irq_action_t action, void* arg)
{
  if(irq >= IRQ_MAX)
    return -1;

  irq_desc_t* desc = &irq_descs[irq];

  unsigned long flags = raw_spin_lock_irqsave(&desc->rlock);
  desc->action = action;
  desc->arg = arg;
  raw_spin_unlock_irqrestore(&desc->rlock, flags);

  return 0;

}

int irq_unregister_action(uint32_t irq)
{
  if(irq >= IRQ_MAX)
    return -1;

  irq_desc_t* desc = &irq_descs[irq];

  unsigned long flags = raw_spin_lock_irqsave(&desc->rlock);
  desc->action = NULL;
  desc->arg = NULL;
  raw_spin_unlock_irqrestore(&desc->rlock, flags);

  return 0;
}

int irq_enable(uint32_t irq)
{
  if(irq >= IRQ_MAX)
    return -1;

  irq_desc_t* desc = &irq_descs[irq];

  unsigned long flags = raw_spin_lock_irqsave(&desc->rlock);

  if (!desc->chip){
    raw_spin_unlock_irqrestore(&desc->rlock, flags);
    return 1;
  }

  if (desc->chip->unmask)
    desc->chip->unmask(irq);

  desc->enabled = 1;

  raw_spin_unlock_irqrestore(&desc->rlock, flags);
  return 0;
}

int irq_disable(uint32_t irq)
{
  if(irq >= IRQ_MAX)
    return -1;

  irq_desc_t* desc = &irq_descs[irq];

  unsigned long flags = raw_spin_lock_irqsave(&desc->rlock);

  if (!desc->chip){
    raw_spin_unlock_irqrestore(&desc->rlock, flags);

    return 1;
  }

  if (desc->chip->mask)
    desc->chip->mask(irq);

  desc->enabled = 0;

  raw_spin_unlock_irqrestore(&desc->rlock, flags);
  return 0;
}

void irq_dispatch(uint32_t irq)
{
  if(irq >= IRQ_MAX)
    return;

  irq_desc_t* desc = &irq_descs[irq];

  unsigned long flags = raw_spin_lock_irqsave(&desc->rlock);
  irq_handler_t handler = desc->handler;
  raw_spin_unlock_irqrestore(&desc->rlock, flags);

  if (handler) {
    handler(desc);
  }
}

uint32_t irq_get_vector(uint32_t irq)
{
  if(irq >= IRQ_MAX)
    return IRQ_VECTOR_INVALID;

  irq_desc_t* desc = &irq_descs[irq];

  unsigned long flags = raw_spin_lock_irqsave(&desc->rlock);

  uint32_t vector = desc->vector;

  raw_spin_unlock_irqrestore(&desc->rlock, flags);

  return vector;
}
