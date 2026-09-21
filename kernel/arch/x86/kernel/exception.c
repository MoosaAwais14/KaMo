#include <asm/exception.h>

#include <asm/irqflags.h>
#include <asm/cpu.h>

#include <lib/memory.h>

static void arch_early_exception(idt_frame_t* frame);

static void install_exception_vectors(void);

static exception_desc_t exception_descs[ARCH_EXCEPTION_COUNT];

void arch_exception_early_init(void)
{
  memset(exception_descs, 0, sizeof(exception_descs));

  for(size_t vector = 0; vector < ARCH_EXCEPTION_COUNT; vector++)
  {
    exception_desc_t* desc = &exception_descs[vector];

    desc->handler = arch_early_exception;
    desc->flags = 0;
    desc->rlock = RAW_SPINLOCK_UNLOCKED;
  }
}

void arch_exception_dispatch(interrupt_context_t* context)
{
  uint32_t vector = context->vector;

  if(vector >= ARCH_EXCEPTION_COUNT)
    return;

  exception_desc_t* desc = &exception_descs[vector];

  unsigned long flags = raw_spin_lock_irqsave(&desc->rlock);
  exception_handler_t fn = desc->handler;
  raw_spin_unlock_irqrestore(&desc->rlock, flags);

  if(fn)
    fn(context->arch);
}

static void install_exception_vectors(void)
{
  for(size_t vector = 0; vector < ARCH_EXCEPTION_COUNT; vector++)
  {
    exception_desc_t* desc = &exception_descs[vector];

    unsigned long flags = raw_spin_lock_irqsave(&desc->rlock);
    exception_handler_t fn = desc->handler;
    raw_spin_unlock_irqrestore(&desc->rlock, flags);

    interrupt_set_type(vector, INTERRUPT_EXCEPTION);
  }
}

static void arch_early_exception(idt_frame_t* frame)
{
  for(;;)
  {
    local_irq_disable();
    local_halt();
  }
}
