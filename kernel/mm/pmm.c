#include <mm/pmm.h>

#include <asm/pmm.h>

#include <locking/spinlock.h>

#include <lib/memory.h>
#include <lib/bitmap.h>

#define PMM_BITMAP_BIT_COUNT              \
  ((PMM_MAX_ADDRESS / PMM_FRAME_SIZE) + 1)

#define PMM_BITMAP_WORD_COUNT             \
  BITMAP_WORD_COUNT(PMM_BITMAP_BIT_COUNT)

struct pmm_s {
  BITMAP_WORD_TYPE bitmap_storage[PMM_BITMAP_WORD_COUNT];

  bitmap_t bitmap;
  raw_spinlock_t rlock;
} __aligned(sizeof(unsigned long));

static struct pmm_s pmm = { .rlock = RAW_SPINLOCK_UNLOCKED };

pmm_err_t pmm_init(void)
{ 
  unsigned long flags = raw_spin_lock_irqsave(&pmm.rlock);

  bitmap_init(&pmm.bitmap);
  bitmap_place(&pmm.bitmap, pmm.bitmap_storage);
  bitmap_set_bit_count(&pmm.bitmap, PMM_BITMAP_BIT_COUNT);

  memset(pmm.bitmap_storage, 0xFF, sizeof(pmm.bitmap_storage));

  raw_spin_unlock_irqrestore(&pmm.rlock, flags);

  return PMM_OK;
}
