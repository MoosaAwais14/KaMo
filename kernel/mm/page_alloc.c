#include <mm/page_alloc.h>
#include <mm/memblock.h>
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
};

struct page_alloc_s {
  struct buddy_zone_s zones[ZONE_MAX];
  pfn_t base_pfn;
  page_t* page_array;
  size_t total_pages;
};

static inline void list_add(page_t** head, page_t* page);
static inline void list_remove(page_t** head, page_t* page);

static struct page_alloc_s page_alloc;

page_alloc_err_t page_alloc_init(void)
{
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
  pfn_t end_pfn_excl = PHYS_PFN(ALIGN_DOWN(mem_phys.end + 1, PAGE_SIZE));

  size_t page_count = end_pfn_excl - base_pfn;

  page_alloc.base_pfn = base_pfn;
  page_alloc.total_pages = page_count;

  range_t page_array_range;
  if(memblock_alloc_range(page_alloc.total_pages * sizeof(page_t), _Alignof(page_t), phys_direct_map_range, &page_array_range) != MEMBLOCK_OK)
    return PAGE_ALLOC_ERR_NOMEM;

  page_alloc.page_array = __va(page_array_range.start);
  memset(page_alloc.page_array, 0, page_alloc.total_pages * sizeof(page_t));

  memblock_iter_t ires;
  range_t res_range;
  if (memblock_reserved_first(&ires, &res_range) == MEMBLOCK_OK) 
  {
    do 
    {
      pfn_t res_start = PHYS_PFN(ALIGN_UP(res_range.start, PAGE_SIZE));
      pfn_t res_end = PHYS_PFN(ALIGN_DOWN(res_range.end + 1, PAGE_SIZE));

      for (pfn_t pfn = res_start; pfn < res_end; pfn++) 
      {
        page_t* page = pfn_to_page(pfn);

        if (pfn >= page_alloc.base_pfn && pfn < page_alloc.base_pfn + page_alloc.total_pages) 
        {
          page->refcount = 1;
          // add flag, maybe to indicate it predates page_alloc
        }
      }
    } while (memblock_reserved_next(&ires, &res_range) == MEMBLOCK_OK);
  }

  if (memblock_memory_first(&imem, &mem_range) == MEMBLOCK_OK) 
  {
    do 
    {
      pfn_t mem_start = PHYS_PFN(ALIGN_UP(mem_range.start, PAGE_SIZE));
      pfn_t mem_end = PHYS_PFN(ALIGN_DOWN(mem_range.end + 1, PAGE_SIZE));

      for (pfn_t pfn = mem_start; pfn < mem_end; pfn++) 
      {
        page_t* page = pfn_to_page(pfn);

        if (page->refcount == 0) 
        {
          free_pages(page, 0);
        }
      }
    } while (memblock_memory_next(&imem, &mem_range) == MEMBLOCK_OK);
  }

  return PAGE_ALLOC_OK;
}

page_t* alloc_pages(zone_type_t zone_type, uint8_t order) 
{
  if (order > MAX_PAGE_ORDER || zone_type >= ZONE_MAX)
    return NULL;

  struct buddy_zone_s *zone = &page_alloc.zones[zone_type];
  uint8_t current_order = order;

  while (current_order <= MAX_PAGE_ORDER && !zone->orders[current_order].area.free_list) 
  {
    current_order++;
  }

  if (current_order > MAX_PAGE_ORDER) 
  {
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

    buddy->order = current_order;
    buddy->refcount = 0;

    list_add(&zone->orders[current_order].area.free_list, buddy);
    zone->orders[current_order].area.nr_free++;
    zone->free_pages += (1UL << current_order);
  }

  page->order = order;
  page->refcount = 1;

  return page;
}

void free_pages(page_t* page, uint8_t order) 
{
  if (!page || order > MAX_PAGE_ORDER)
    return;
 
  // figure this out, right now its only on normal.
  struct buddy_zone_s *zone = &page_alloc.zones[ZONE_NORMAL]; 
  pfn_t pfn = page_to_pfn(page);

  while (order < MAX_PAGE_ORDER) 
  {
    pfn_t buddy_pfn = pfn ^ (1UL << order);

    if (buddy_pfn < page_alloc.base_pfn ||  buddy_pfn >= page_alloc.base_pfn + page_alloc.total_pages) 
    {
      break;
    }

    page_t *buddy = pfn_to_page(buddy_pfn);

    if (buddy->refcount != 0 || buddy->order != order) 
    {
      break;
    }

    list_remove(&zone->orders[order].area.free_list, buddy);
    zone->orders[order].area.nr_free--;

    if (buddy_pfn < pfn) 
    {
      pfn = buddy_pfn;
      page = buddy;
    }

    order++;
  }

  page->order = order;
  page->refcount = 0;

  list_add(&zone->orders[order].area.free_list, page);
  zone->orders[order].area.nr_free++;
  zone->free_pages += (1UL << order);
}

page_t* pfn_to_page(pfn_t pfn) 
{
  return &page_alloc.page_array[pfn - page_alloc.base_pfn];
}

pfn_t page_to_pfn(page_t* page) 
{
  return page_alloc.base_pfn + (size_t)(page - page_alloc.page_array);
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
