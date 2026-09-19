#ifndef KERNEL_INCLUDE_SYNC_SPINLOCK_H
#define KERNEL_INCLUDE_SYNC_SPINLOCK_H

#include <stdint.h>
#include <stdatomic.h>

typedef struct raw_spinlock_s {
  _Atomic uint8_t lock;
} raw_spinlock_t;

#define RAW_SPINLOCK_UNLOCKED (raw_spinlock_t){ .lock = 0 }

// typedef struct spinlock_s {
//   raw_spinlock_t raw;
// } spinlock_t;

extern void raw_spin_lock(raw_spinlock_t* lock);
extern void raw_spin_unlock(raw_spinlock_t* lock);

extern unsigned long raw_spin_lock_irqsave(raw_spinlock_t* lock);
extern void raw_spin_unlock_irqrestore(raw_spinlock_t* lock, unsigned long flags);

#endif
