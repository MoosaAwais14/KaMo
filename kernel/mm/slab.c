#include <mm/slab.h>

#include <asm/page.h>
#include <mm/page_alloc.h>

#include <locking/spinlock.h>

#include <lib/memory.h>

struct free_slab_s {
  struct free_slab_s *next;
};

struct cache_s {
  struct free_slab_s* free_list;
  raw_spinlock_t rlock;
};

struct slab_s {
  struct cache_s cache[SLAB_INDEX_COUNT];
};

static inline slab_index_t get_slab_index(size_t size);
static inline size_t get_slab_size(slab_index_t slab_index);

static void create_slab(slab_index_t slab_index);

static struct slab_s slab;

void slab_init(void)
{
  memset(&slab, 0, sizeof(struct slab_s));
  for(size_t i = 0; i < SLAB_INDEX_COUNT; i++)
    slab.cache[i].rlock = RAW_SPINLOCK_UNLOCKED;
}

void* kmalloc(size_t size) {
  slab_index_t slab_index = get_slab_index(size);
  if (slab_index == SLAB_INDEX_COUNT) return NULL;

  struct cache_s* cache = &slab.cache[slab_index];
  unsigned long irq_flags = raw_spin_lock_irqsave(&cache->rlock);

  struct free_slab_s* slab = cache->free_list;
  if (!slab) 
  {
    create_slab(slab_index);
    if (!cache->free_list) 
    {
      raw_spin_unlock_irqrestore(&cache->rlock, irq_flags);
      return NULL;
    }
    slab = cache->free_list;
  }
  else
  {
    page_t* page = pfn_to_page(PHYS_PFN(___pa(slab)));
    if (page) 
    {
      page->refcount++;
    }
  }

  cache->free_list = slab->next;

  raw_spin_unlock_irqrestore(&cache->rlock, irq_flags);

  return (void*)slab;
}

void kfree(void* ptr)
{
  if (!ptr)
    return;

  pfn_t pfn = PHYS_PFN(__pa(ptr));
  page_t* page = pfn_to_page(pfn);

  if (!page || !(page->flags & PAGE_FLAG_SLAB_ALLOCATION))
    return;

  slab_index_t slab_index = page->slab_index;
  
  struct cache_s* cache = &slab.cache[slab_index];
  unsigned long irq_flags = raw_spin_lock_irqsave(&cache->rlock);

  struct free_slab_s* slab = (struct free_slab_s*)ptr;
  slab->next = cache->free_list;
  cache->free_list = slab;

  if (page->refcount > 0)
    page->refcount--;

  if (page->refcount == 0)
    free_pages(page, page->order);

  raw_spin_unlock_irqrestore(&cache->rlock, irq_flags);
}

static void create_slab(slab_index_t slab_index)
{
  size_t slab_size = get_slab_size(slab_index);
  int order = get_order(slab_size);
  if (order == -1) return;

  page_t* pages = alloc_pages(ZONE_NORMAL, (uint8_t)order); 
  if (!pages) return;

  size_t num_pages = 1UL << order;
  for (size_t i = 0; i < num_pages; i++) 
  {
    pages[i].slab_index = slab_index;
    pages[i].flags |= PAGE_FLAG_SLAB_ALLOCATION;
  }

  struct cache_s* cache = &slab.cache[slab_index];

  pfn_t pfn = page_to_pfn(pages);
  phys_addr_t phys = PFN_PHYS(pfn);
  uint8_t* curr = __va(phys);
  uint8_t* end = curr + order_to_size((uint8_t)order);

  while (curr + slab_size <= end) 
  {
    struct free_slab_s* slab = (struct free_slab_s*)curr;
    slab->next = cache->free_list;
    cache->free_list = slab;
    curr += slab_size;
  }
}

static inline slab_index_t get_slab_index(size_t size)
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

static inline size_t get_slab_size(slab_index_t slab_index)
{
  return (16UL << slab_index);
}
