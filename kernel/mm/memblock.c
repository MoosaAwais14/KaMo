#include <mm/memblock.h>

#include <sync/spinlock.h>

#include <lib/bitmap.h>

struct memblock_state_s {
  uint8_t reserved[MEMBLOCK_RESERVED_SIZE];
  
  void* pool_start;
  void* pool_end;

  bitmap_t bitmap;

  raw_spinlock_t rlock;
};

static inline size_t size_to_region_count(size_t size);

static int get_free_region(size_t target, size_t aligned, size_t* out);

static struct memblock_state_s memblock = {
  .rlock = RAW_SPINLOCK_UNLOCKED
};

void memblock_init(void)
{
  unsigned long flags = raw_spin_lock_irqsave(&memblock.rlock);

  bitmap_init(&memblock.bitmap);
  bitmap_place(&memblock.bitmap, memblock.reserved);
  bitmap_set_bit_count(&memblock.bitmap, size_to_region_count(MEMBLOCK_RESERVED_SIZE));

  memblock.pool_start = bitmap_array_end(&memblock.bitmap);
  memblock.pool_end = &memblock.reserved[MEMBLOCK_RESERVED_SIZE - 1];

  bitmap_clear(&memblock.bitmap);

  raw_spin_unlock_irqrestore(&memblock.rlock, flags);
}

void* memblock_alloc(size_t size, size_t aligned)
{
  unsigned long flags = raw_spin_lock_irqsave(&memblock.rlock);

  size_t region_count = size_to_region_count(size);
  size_t region;

  if(get_free_region(region_count, aligned, &region))
  {
    raw_spin_unlock_irqrestore(&memblock.rlock, flags);
    return NULL;
  }

  for (size_t i = 0; i < region_count; i++)
  {
    bitmap_set_bit(&memblock.bitmap, region + i);
  }

  raw_spin_unlock_irqrestore(&memblock.rlock, flags);
  return (memblock.pool_start + (MEMBLOCK_REGION_WIDTH * region));
}

static int get_free_region(size_t target, size_t aligned, size_t *out)
{
  if (!out || target == 0)
    return -1;

  size_t count = 0;

  for (size_t region = 0; region < memblock.bitmap.bit_count; region++)
  {
    if(count == 0 && aligned != 0)
    {
      uintptr_t addr = ((uintptr_t)memblock.pool_start + (MEMBLOCK_REGION_WIDTH * region));
      if(addr % aligned != 0)
        continue;
    }

    if (bitmap_test_bit(&memblock.bitmap, region))
    {
      count = 0;
      continue;
    }

    count++;

    if (count == target)
    {
      *out = region - target + 1;
      return 0;
    }
  }

  return 1;
}

static inline size_t size_to_region_count(size_t size)
{
  return (ALIGN_UP(size, MEMBLOCK_REGION_WIDTH)) / MEMBLOCK_REGION_WIDTH;
}
