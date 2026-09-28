#include <mm/membump.h>

#include <compiler/attributes.h>
#include <lib/sizes.h>

#include <locking/spinlock.h>
#include <lib/memory.h>

struct membump_s {
  uint8_t reserved[MEMBUMP_RESERVED_SIZE];

  uint8_t* brk;
  uint8_t* brk_limit;

  raw_spinlock_t rlock;
} __aligned(KiB(4));

static struct membump_s membump = {
  .rlock = RAW_SPINLOCK_UNLOCKED
};

void membump_init(void)
{
  unsigned long flags = raw_spin_lock_irqsave(&membump.rlock);
    
  membump.brk = membump.reserved;
  membump.brk_limit = membump.reserved + sizeof(membump.reserved);

  raw_spin_unlock_irqrestore(&membump.rlock, flags);
}

void* membump_alloc(size_t size, size_t aligned)
{
  unsigned long flags = raw_spin_lock_irqsave(&membump.rlock);

  uint8_t* target = (uint8_t*)ALIGN_UP((uintptr_t)membump.brk, aligned);
  uint8_t* target_end = (target + size);

  if(target_end > membump.brk_limit)
  {
    raw_spin_unlock_irqrestore(&membump.rlock, flags);
    return NULL;
  }
  
  membump.brk = (uint8_t*)target_end;
  raw_spin_unlock_irqrestore(&membump.rlock, flags);
  return (void*)target;
}
