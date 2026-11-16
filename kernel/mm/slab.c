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

static void create_slab(slab_index_t slab_index);
static void remove_slab_free_objects(struct cache_s* cache, page_t* head);

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

  cache->free_list = slab->next;

  page_t* page = pfn_to_page(PHYS_PFN(___pa(slab)));
  page_t* head = page ? page->compound_head : NULL;
  if (!head || !(head->flags & PAGE_FLAG_SLAB_ALLOCATION) || head->slab_index != slab_index)
  {
    raw_spin_unlock_irqrestore(&cache->rlock, irq_flags);
    return NULL;
  }
  head->refcount++;

  raw_spin_unlock_irqrestore(&cache->rlock, irq_flags);

  return (void*)slab;
}

void kfree(void* ptr)
{
  if (!ptr)
    return;

  pfn_t pfn = PHYS_PFN(__pa(ptr));
  page_t* page = pfn_to_page(pfn);
  page_t* head = page ? page->compound_head : NULL;
  if (!head || !(head->flags & PAGE_FLAG_SLAB_ALLOCATION))
    return;

  slab_index_t slab_index = (slab_index_t)head->slab_index;
  if (slab_index >= SLAB_INDEX_COUNT)
    return;
  
  struct cache_s* cache = &slab.cache[slab_index];
  unsigned long irq_flags = raw_spin_lock_irqsave(&cache->rlock);

  if (head->refcount == 0)
  {
    raw_spin_unlock_irqrestore(&cache->rlock, irq_flags);
    return;
  }

  struct free_slab_s* slab = (struct free_slab_s*)ptr;
  slab->next = cache->free_list;
  cache->free_list = slab;
  head->refcount--;
  if (head->refcount == 0)
  {
    remove_slab_free_objects(cache, head);
    head->refcount = 1;
    free_pages(head, head->order);
  }

  raw_spin_unlock_irqrestore(&cache->rlock, irq_flags);
}

static void create_slab(slab_index_t slab_index)
{
  size_t slab_size = get_slab_size(slab_index);
  int order = get_order(slab_size);
  if (order == -1) return;

  page_t* pages = alloc_pages(ZONE_NORMAL, (uint8_t)order); 
  if (!pages) return;

  pages->slab_index = slab_index;
  pages->flags |= PAGE_FLAG_SLAB_ALLOCATION;
  pages->refcount = 0;

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

static void remove_slab_free_objects(struct cache_s* cache, page_t* head)
{
  uintptr_t slab_start = (uintptr_t)__va(PFN_PHYS(page_to_pfn(head)));
  uintptr_t slab_end = slab_start + order_to_size(head->order);
  struct free_slab_s** link = &cache->free_list;

  while (*link)
  {
    struct free_slab_s* object = *link;
    uintptr_t address = (uintptr_t)object;
    if (address >= slab_start && address < slab_end)
    {
      *link = object->next;
      continue;
    }

    link = &object->next;
  }
}
