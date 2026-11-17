#include <mm/page_alloc.h>
#include <mm/memblock.h>
#include <locking/spinlock.h>
#include <lib/memory.h>
#include <lib/string.h>

#define NR_PAGE_ORDERS (MAX_PAGE_ORDER + 1)

struct free_area_s {
  page_t* free_list;
  size_t nr_free;
};

struct order_s {
  struct free_area_s area;
};

struct buddy_zone_s {
  zone_type_t type;
  pfn_t base_pfn;
  size_t free_pages;
  struct order_s orders[NR_PAGE_ORDERS];
  raw_spinlock_t rlock;
};

struct page_alloc_s {
  struct buddy_zone_s zones[ZONE_MAX];
  pfn_t base_pfn;
  page_t* page_array;
  size_t total_pages;
};

static inline void list_add(page_t** head, page_t* page);
static inline void list_remove(page_t** head, page_t* page);
static void set_page_block(page_t* head, uint8_t order, page_state_t head_state, page_state_t tail_state);
static int page_pointer_to_pfn(page_t* page, pfn_t* pfn);
static page_ref_err_t page_put_internal(page_t* page, int check_order, uint8_t order);

static struct buddy_zone_s* get_zone(pfn_t pfn);

static struct page_alloc_s page_alloc;

page_alloc_err_t page_alloc_init(void)
{
  for(size_t i = 0; i < ZONE_MAX; i++)
    page_alloc.zones[i].rlock = RAW_SPINLOCK_UNLOCKED;

  memblock_iter_t imem;
  range_t mem_range;

  if(memblock_memory_first(&imem, &mem_range) != MEMBLOCK_OK)
    return PAGE_ALLOC_ERR_INVALID;

  range_t mem_phys = { mem_range.start, mem_range.end };

  while(memblock_memory_next(&imem, &mem_range) == MEMBLOCK_OK)
  {
    mem_phys.end = mem_range.end;
  }

  pfn_t base_pfn = PHYS_PFN(ALIGN_UP(mem_phys.start, PAGE_SIZE));
  pfn_t end_pfn_excl = PHYS_PFN(mem_phys.end) + ((mem_phys.end & (PAGE_SIZE - 1)) == PAGE_SIZE - 1);

  size_t page_count = end_pfn_excl - base_pfn;
  if (page_count == 0 || page_count > SIZE_MAX / sizeof(page_t))
    return PAGE_ALLOC_ERR_INVALID;

  page_alloc.base_pfn = base_pfn;
  page_alloc.total_pages = page_count;

  range_t page_array_range;
  if(memblock_alloc_range(page_alloc.total_pages * sizeof(page_t), 0, phys_direct_map_range, &page_array_range) != MEMBLOCK_OK)
    return PAGE_ALLOC_ERR_NOMEM;

  page_alloc.page_array = __va(page_array_range.start);
  memset(page_alloc.page_array, 0, page_alloc.total_pages * sizeof(page_t));

  if (memblock_memory_first(&imem, &mem_range) == MEMBLOCK_OK)
  {
    do
    {
      pfn_t mem_start = PHYS_PFN(ALIGN_UP(mem_range.start, PAGE_SIZE));
      pfn_t mem_end = PHYS_PFN(mem_range.end) + ((mem_range.end & (PAGE_SIZE - 1)) == PAGE_SIZE - 1);

      for (pfn_t pfn = mem_start; pfn < mem_end; pfn++)
      {
        page_t* page = pfn_to_page(pfn);
        if (!page)
          continue;

        page->state = PAGE_STATE_ALLOCATED_HEAD;
        page->compound_head = page;
        page->refcount = 1;
      }
    } while (memblock_memory_next(&imem, &mem_range) == MEMBLOCK_OK);
  }

  memblock_iter_t ires;
  range_t res_range;
  if (memblock_reserved_first(&ires, &res_range) == MEMBLOCK_OK)
  {
    do
    {
      pfn_t res_start = PHYS_PFN(res_range.start);
      pfn_t res_end = PHYS_PFN(res_range.end) + 1;

      for (pfn_t pfn = res_start; pfn < res_end; pfn++)
      {
        page_t* page = pfn_to_page(pfn);
        if (page && page->state != PAGE_STATE_INVALID)
        {
          page->state = PAGE_STATE_RESERVED;
          page->compound_head = page;
          page->refcount = 0;
        }
      }
    } while (memblock_reserved_next(&ires, &res_range) == MEMBLOCK_OK);
  }

  for (pfn_t pfn = base_pfn; pfn < end_pfn_excl; pfn++)
  {
    page_t* page = pfn_to_page(pfn);
    if (page && page->state == PAGE_STATE_ALLOCATED_HEAD)
      free_pages(page, 0);
  }

  return PAGE_ALLOC_OK;
}

page_t* alloc_pages(zone_type_t zone_type, uint8_t order) 
{
  if (order > MAX_PAGE_ORDER || zone_type >= ZONE_MAX)
    return NULL;

  struct buddy_zone_s *zone = &page_alloc.zones[zone_type];
  uint8_t current_order = order;

  unsigned long irq_flags = raw_spin_lock_irqsave(&zone->rlock);

  while (current_order <= MAX_PAGE_ORDER && !zone->orders[current_order].area.free_list) 
  {
    current_order++;
  }

  if (current_order > MAX_PAGE_ORDER) 
  {
    raw_spin_unlock_irqrestore(&zone->rlock, irq_flags);
    return NULL; 
  }

  page_t *page = zone->orders[current_order].area.free_list;
  list_remove(&zone->orders[current_order].area.free_list, page);
  zone->orders[current_order].area.nr_free--;
  zone->free_pages -= (1UL << current_order);

  while (current_order > order) 
  {
    current_order--;

    pfn_t buddy_pfn = page_to_pfn(page) + (1UL << current_order);
    page_t *buddy = pfn_to_page(buddy_pfn);

    set_page_block(buddy, current_order, PAGE_STATE_FREE_HEAD, PAGE_STATE_FREE_TAIL);

    list_add(&zone->orders[current_order].area.free_list, buddy);
    zone->orders[current_order].area.nr_free++;
    zone->free_pages += (1UL << current_order);
  }

  set_page_block(page, order, PAGE_STATE_ALLOCATED_HEAD, PAGE_STATE_ALLOCATED_TAIL);
  page->order = order;
  page->refcount = 1;
  page->flags = PAGE_FLAG_GENERIC;

  raw_spin_unlock_irqrestore(&zone->rlock, irq_flags);
  return page;
}

page_ref_err_t page_get(page_t* page)
{
  pfn_t pfn;
  if (!page_pointer_to_pfn(page, &pfn))
    return PAGE_REF_ERR_INVALID;

  page_t* member = pfn_to_page(pfn);
  struct buddy_zone_s* zone = get_zone(pfn);
  if (!member || !zone)
    return PAGE_REF_ERR_INVALID;

  unsigned long irq_flags = raw_spin_lock_irqsave(&zone->rlock);
  page_t* head = member->compound_head;
  pfn_t head_pfn;
  if (!page_pointer_to_pfn(head, &head_pfn) ||
      get_zone(head_pfn) != zone ||
      head->state != PAGE_STATE_ALLOCATED_HEAD ||
      head->compound_head != head ||
      head->order > MAX_PAGE_ORDER ||
      member->state != (member == head ? PAGE_STATE_ALLOCATED_HEAD : PAGE_STATE_ALLOCATED_TAIL) ||
      pfn < head_pfn || pfn - head_pfn >= (1UL << head->order) ||
      head->refcount == 0)
  {
    raw_spin_unlock_irqrestore(&zone->rlock, irq_flags);
    return PAGE_REF_ERR_INVALID;
  }

  if (head->refcount == SIZE_MAX)
  {
    raw_spin_unlock_irqrestore(&zone->rlock, irq_flags);
    return PAGE_REF_ERR_OVERFLOW;
  }

  head->refcount++;
  raw_spin_unlock_irqrestore(&zone->rlock, irq_flags);
  return PAGE_REF_OK;
}

page_ref_err_t page_put(page_t* page)
{
  return page_put_internal(page, 0, 0);
}

void free_pages(page_t* page, uint8_t order)
{
  if (order > MAX_PAGE_ORDER)
    return;

  (void)page_put_internal(page, 1, order);
}

static page_ref_err_t page_put_internal(page_t* page, int check_order, uint8_t expected_order)
{
  pfn_t pfn;
  if (!page_pointer_to_pfn(page, &pfn))
    return PAGE_REF_ERR_INVALID;

  page_t* member = pfn_to_page(pfn);
  struct buddy_zone_s* zone = get_zone(pfn);
  if (!member || !zone)
    return PAGE_REF_ERR_INVALID;

  unsigned long irq_flags = raw_spin_lock_irqsave(&zone->rlock);
  page_t* head = member->compound_head;
  pfn_t head_pfn;

  if (!page_pointer_to_pfn(head, &head_pfn) ||
      get_zone(head_pfn) != zone ||
      head->state != PAGE_STATE_ALLOCATED_HEAD ||
      head->compound_head != head ||
      head->order > MAX_PAGE_ORDER ||
      member->state != (member == head ? PAGE_STATE_ALLOCATED_HEAD : PAGE_STATE_ALLOCATED_TAIL) ||
      pfn < head_pfn || pfn - head_pfn >= (1UL << head->order) ||
      head->refcount == 0 ||
      (check_order && (member != head || head->order != expected_order)))
  {
    raw_spin_unlock_irqrestore(&zone->rlock, irq_flags);
    return PAGE_REF_ERR_INVALID;
  }

  head->refcount--;
  if (head->refcount != 0)
  {
    raw_spin_unlock_irqrestore(&zone->rlock, irq_flags);
    return PAGE_REF_OK;
  }

  uint8_t order = head->order;
  const size_t released_pages = 1UL << order;
  page_t* free_head = head;
  pfn_t free_pfn = head_pfn;

  while (order < MAX_PAGE_ORDER) 
  {
    pfn_t buddy_pfn = free_pfn ^ (1UL << order);

    if (buddy_pfn < page_alloc.base_pfn ||  buddy_pfn >= page_alloc.base_pfn + page_alloc.total_pages) 
    {
      break;
    }

    page_t *buddy = pfn_to_page(buddy_pfn);

    if (!buddy || get_zone(buddy_pfn) != zone || buddy->state != PAGE_STATE_FREE_HEAD || buddy->order != order)
    {
      break;
    }

    list_remove(&zone->orders[order].area.free_list, buddy);
    zone->orders[order].area.nr_free--;

    if (buddy_pfn < free_pfn)
    {
      free_pfn = buddy_pfn;
      free_head = buddy;
    }

    order++;
  }

  set_page_block(free_head, order, PAGE_STATE_FREE_HEAD, PAGE_STATE_FREE_TAIL);
  free_head->order = order;

  list_add(&zone->orders[order].area.free_list, free_head);
  zone->orders[order].area.nr_free++;
  zone->free_pages += released_pages;

  raw_spin_unlock_irqrestore(&zone->rlock, irq_flags);
  return PAGE_REF_OK;
}

page_t* pfn_to_page(pfn_t pfn) 
{
  if(pfn >= (page_alloc.total_pages + page_alloc.base_pfn) || pfn < page_alloc.base_pfn)
    return NULL;
  return &page_alloc.page_array[pfn - page_alloc.base_pfn];
}

pfn_t page_to_pfn(page_t* page) 
{
  return page_alloc.base_pfn + (size_t)(page - page_alloc.page_array);
}

static struct buddy_zone_s* get_zone(pfn_t pfn)
{
  pfn_t direct_map_pfn_start = PHYS_PFN(phys_direct_map_range.start);
  pfn_t direct_map_pfn_end = PHYS_PFN(phys_direct_map_range.end + 1);
  
  if(pfn < direct_map_pfn_start)
    return NULL;

  if(pfn < direct_map_pfn_end)
    return &page_alloc.zones[ZONE_NORMAL];

  return &page_alloc.zones[ZONE_HIGHMEM];
}

static int page_pointer_to_pfn(page_t* page, pfn_t* pfn)
{
  if (!page || !pfn || !page_alloc.page_array)
    return 0;

  uintptr_t base = (uintptr_t)page_alloc.page_array;
  uintptr_t address = (uintptr_t)page;
  if (address < base)
    return 0;

  uintptr_t offset = address - base;
  if (offset % sizeof(page_t) != 0 || offset / sizeof(page_t) >= page_alloc.total_pages)
    return 0;

  *pfn = page_alloc.base_pfn + offset / sizeof(page_t);
  return 1;
}

static void set_page_block(page_t* head, uint8_t order, page_state_t head_state, page_state_t tail_state)
{
  const size_t page_count = 1UL << order;

  for (size_t i = 0; i < page_count; i++)
  {
    page_t* page = pfn_to_page(page_to_pfn(head) + i);
    if (!page)
      continue;

    page->prev = NULL;
    page->next = NULL;
    page->compound_head = head;
    page->refcount = 0;
    page->order = 0;
    page->state = i == 0 ? head_state : tail_state;
    page->flags = 0;
  }

  head->order = order;
}

static inline void list_add(page_t** head, page_t* page) 
{
  page->next = *head;
  page->prev = NULL;
  if (*head) 
  {
    (*head)->prev = page;
  }
  *head = page;
}

static inline void list_remove(page_t** head, page_t* page) 
{
  if (page->prev) 
  {
    page->prev->next = page->next;
  } 
  else 
  {
    *head = page->next;
  }

  if (page->next) 
  {
    page->next->prev = page->prev;
  }

  page->next = NULL;
  page->prev = NULL;
}
