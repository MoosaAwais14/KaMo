#include <mm/slab.h>

#include <asm/page.h>
#include <mm/page_alloc.h>

#include <locking/spinlock.h>

#include <lib/memory.h>

typedef enum slab_index_e {
  SLAB_INDEX_16 = 0,
  SLAB_INDEX_32,
  SLAB_INDEX_64,
  SLAB_INDEX_128,
  SLAB_INDEX_256,
  SLAB_INDEX_512,
  SLAB_INDEX_1024,
  SLAB_INDEX_2048,
  SLAB_INDEX_4096,
  SLAB_INDEX_8192,
  SLAB_INDEX_COUNT
} slab_index_t;

static slab_index_t get_slab_index(size_t size)
{
  if (size <= 16)   return SLAB_INDEX_16;
  if (size <= 32)   return SLAB_INDEX_32;
  if (size <= 64)   return SLAB_INDEX_64;
  if (size <= 128)  return SLAB_INDEX_128;
  if (size <= 256)  return SLAB_INDEX_256;
  if (size <= 512)  return SLAB_INDEX_512;
  if (size <= 1024) return SLAB_INDEX_1024;
  if (size <= 2048) return SLAB_INDEX_2048;
  if (size <= 4096) return SLAB_INDEX_4096;
  if (size <= 8192) return SLAB_INDEX_8192;
  return SLAB_INDEX_COUNT;
}

static size_t get_slab_size(slab_index_t slab_index)
{
  return (16UL << slab_index);
}

#define SLAB_MAX_OBJECTS (PAGE_SIZE / 16)
#define SLAB_BITMAP_WORDS ((SLAB_MAX_OBJECTS + 31) / 32)

struct slab_desc_s {
  struct slab_desc_s* next;
  struct slab_desc_s* free_next;
  page_t* pages;
  uintptr_t start;
  size_t object_size;
  size_t object_count;
  size_t inuse;
  uint32_t allocated[SLAB_BITMAP_WORDS];
};

struct cache_s {
  struct slab_desc_s* slabs;
};

struct slab_s {
  struct cache_s cache[SLAB_INDEX_COUNT];
  struct slab_desc_s* free_descriptors;
  raw_spinlock_t rlock;
};

static struct slab_s slab;

static struct slab_desc_s* alloc_slab_desc(void);
static void free_slab_desc(struct slab_desc_s* desc);

static struct slab_desc_s* create_slab(slab_index_t slab_index);

void slab_init(void)
{
  memset(&slab, 0, sizeof(slab));
  slab.rlock = RAW_SPINLOCK_UNLOCKED;
}

void* kmalloc(size_t size)
{
  slab_index_t slab_index = get_slab_index(size);
  if (slab_index == SLAB_INDEX_COUNT)
    return NULL;

  unsigned long irq_flags = raw_spin_lock_irqsave(&slab.rlock);
  struct cache_s* cache = &slab.cache[slab_index];

  struct slab_desc_s* desc = cache->slabs;
  while (desc && desc->inuse == desc->object_count)
    desc = desc->next;

  if (!desc)
  {
    desc = create_slab(slab_index);
    if (!desc)
    {
      raw_spin_unlock_irqrestore(&slab.rlock, irq_flags);
      return NULL;
    }

    desc->next = cache->slabs;
    cache->slabs = desc;
  }

  for (size_t i = 0; i < desc->object_count; i++)
  {
    size_t word = i / 32;
    uint32_t mask = 1u << (i % 32);
    if (!(desc->allocated[word] & mask))
    {
      desc->allocated[word] |= mask;
      desc->inuse++;
      void* object = (void*)(desc->start + i * desc->object_size);
      raw_spin_unlock_irqrestore(&slab.rlock, irq_flags);
      return object;
    }
  }

  raw_spin_unlock_irqrestore(&slab.rlock, irq_flags);
  return NULL;
}

void kfree(void* ptr)
{
  if (!ptr)
    return;

  uintptr_t address = (uintptr_t)ptr;
  unsigned long irq_flags = raw_spin_lock_irqsave(&slab.rlock);

  for (size_t cache_index = 0; cache_index < SLAB_INDEX_COUNT; cache_index++)
  {
    struct cache_s* cache = &slab.cache[cache_index];
    struct slab_desc_s** link = &cache->slabs;

    while (*link)
    {
      struct slab_desc_s* desc = *link;
      size_t slab_bytes = order_to_size(desc->pages->order);
      if (address < desc->start || address - desc->start >= slab_bytes)
      {
        link = &desc->next;
        continue;
      }

      size_t offset = address - desc->start;
      if (offset % desc->object_size != 0)
      {
        raw_spin_unlock_irqrestore(&slab.rlock, irq_flags);
        return;
      }

      size_t object_index = offset / desc->object_size;
      if (object_index >= desc->object_count)
      {
        raw_spin_unlock_irqrestore(&slab.rlock, irq_flags);
        return;
      }

      size_t word = object_index / 32;
      uint32_t mask = 1u << (object_index % 32);
      if (!(desc->allocated[word] & mask))
      {
        raw_spin_unlock_irqrestore(&slab.rlock, irq_flags);
        return;
      }

      desc->allocated[word] &= ~mask;
      desc->inuse--;
      if (desc->inuse == 0)
      {
        page_t* pages = desc->pages;
        uint8_t order = pages->order;
        *link = desc->next;
        free_pages(pages, order);
        free_slab_desc(desc);
      }

      raw_spin_unlock_irqrestore(&slab.rlock, irq_flags);
      return;
    }
  }

  raw_spin_unlock_irqrestore(&slab.rlock, irq_flags);
}

static struct slab_desc_s* create_slab(slab_index_t slab_index)
{
  size_t object_size = get_slab_size(slab_index);
  int order = get_order(object_size);
  if (order < 0)
    return NULL;

  struct slab_desc_s* desc = alloc_slab_desc();
  if (!desc)
    return NULL;

  page_t* pages = alloc_pages(ZONE_NORMAL, (uint8_t)order);
  if (!pages)
  {
    free_slab_desc(desc);
    return NULL;
  }

  desc->pages = pages;
  desc->start = (uintptr_t)__va(PFN_PHYS(page_to_pfn(pages)));
  desc->object_size = object_size;
  desc->object_count = order_to_size((uint8_t)order) / object_size;
  if (desc->object_count > SLAB_MAX_OBJECTS)
  {
    free_pages(pages, (uint8_t)order);
    free_slab_desc(desc);
    return NULL;
  }

  return desc;
}

static struct slab_desc_s* alloc_slab_desc(void)
{
  if (!slab.free_descriptors)
  {
    page_t* page = alloc_pages(ZONE_NORMAL, 0);
    if (!page)
      return NULL;

    struct slab_desc_s* descriptors = __va(PFN_PHYS(page_to_pfn(page)));
    size_t count = PAGE_SIZE / sizeof(*descriptors);
    if (count == 0)
    {
      free_pages(page, 0);
      return NULL;
    }

    for (size_t i = 0; i < count; i++)
    {
      descriptors[i].free_next = slab.free_descriptors;
      slab.free_descriptors = &descriptors[i];
    }
  }

  struct slab_desc_s* desc = slab.free_descriptors;
  slab.free_descriptors = desc->free_next;
  memset(desc, 0, sizeof(*desc));
  return desc;
}

static void free_slab_desc(struct slab_desc_s* desc)
{
  desc->free_next = slab.free_descriptors;
  slab.free_descriptors = desc;
}