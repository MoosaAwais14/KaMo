#include <sync/spinlock.h>

#include <asm/cpu.h>
#include <asm/irqflags.h>
#include <cpu.h>

void raw_spin_lock(raw_spinlock_t* lock)
{
  for (;;) {
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

unsigned long raw_spin_lock_irqsave(raw_spinlock_t* lock)
{
  unsigned long flags = local_irq_save();
  preempt_enable();
  raw_spin_lock(lock);
  return flags;
}

void raw_spin_unlock_irqrestore(raw_spinlock_t* lock, unsigned long flags)
{
  raw_spin_unlock(lock);
  local_restore_flags(flags);
  preempt_disable();
}
