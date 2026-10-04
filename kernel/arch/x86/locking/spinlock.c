#include <locking/spinlock.h>

#include <asm/cpu.h>
#include <asm/irqflags.h>
#include <kernel/cpu.h>

void raw_spin_lock(raw_spinlock_t* lock)
{
  for (;;) 
  {
    uint8_t expected = 0;

    if (atomic_compare_exchange_weak_explicit(
      &lock->lock,
      &expected,
      1,
      memory_order_acquire,
      memory_order_relaxed)) {
      return;
    }

    while (atomic_load_explicit(
      &lock->lock,
      memory_order_relaxed)) {
      cpu_relax();
    }
  }
}

void raw_spin_unlock(raw_spinlock_t* lock)
{
  atomic_store_explicit(
    &lock->lock,
    0,
    memory_order_release
  );
}

uint8_t raw_spin_trylock(raw_spinlock_t* lock)
{
  uint8_t expected = 0;

  return atomic_compare_exchange_strong_explicit(
    &lock->lock,
    &expected,
    1,
    memory_order_acquire,
    memory_order_relaxed
  );
}

uint8_t raw_spinlock_test(raw_spinlock_t* lock)
{
  return atomic_load_explicit(
    &lock->lock,
    memory_order_relaxed
  );
}

unsigned long raw_spin_lock_irqsave(raw_spinlock_t* lock)
{
  unsigned long flags = local_irq_save();
  preempt_enable();
  raw_spin_lock(lock);
  return flags;
}

uint8_t raw_spin_trylock_irqsave(raw_spinlock_t* lock, unsigned long* flags)
{
  *flags = local_irq_save();
  preempt_enable();

  if (!raw_spin_trylock(lock))
  {
    local_restore_flags(*flags);
    preempt_disable();
    return 0;
  }

  return 1;
}

void raw_spin_unlock_irqrestore(raw_spinlock_t* lock, unsigned long flags)
{
  raw_spin_unlock(lock);
  local_restore_flags(flags);
  preempt_disable();
}
