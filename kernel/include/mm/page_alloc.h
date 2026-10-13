#ifndef MM_PAGE_ALLOC_H
#define MM_PAGE_ALLOC_H

#include <stdint.h>
#include <stddef.h>

#include <kernel/address.h>
#include <asm/page_alloc.h>

typedef enum {
  ZONE_NORMAL = 0,
  ZONE_HIGHMEM,
  ZONE_MAX
} zone_type_t;

typedef enum {
  PAGE_ALLOC_OK = 0,
  PAGE_ALLOC_ERR_NOMEM,
  PAGE_ALLOC_ERR_INVALID
} page_alloc_err_t;

typedef struct page_s {
  struct page_s* prev;
  struct page_s* next; 
  size_t refcount;
  uint8_t order;
  int slab_index;
  uint32_t flags;
} page_t;

#define PAGE_FLAG_UNKOWN          (1u << 0)
#define PAGE_FLAG_MMU_ALLOCATION  (1u << 1)
#define PAGE_FLAG_SLAB_ALLOCATION (1u << 2)

typedef unsigned long pfn_t;

extern page_alloc_err_t page_alloc_init(void);

extern page_t* alloc_pages(zone_type_t zone_type, uint8_t order);
extern void free_pages(page_t* page, uint8_t order);

extern page_t* pfn_to_page(pfn_t pfn);
extern pfn_t page_to_pfn(page_t* page);

static inline int get_order(size_t size)
{
  unsigned long pages = (size + PAGE_SIZE - 1) >> PAGE_SHIFT;
  int order = 0;

  if (pages > 1) {
    pages--;
    while (pages > 0) {
      order++;
      pages >>= 1;
    }
  }

  if (order > MAX_PAGE_ORDER) {
    return -1;
  }

  return order;
}

static inline size_t order_to_size(uint8_t order)
{
  return (PAGE_SIZE << order);
}

#define PHYS_PFN(phys)    ((phys) >> PAGE_SHIFT)
#define PFN_PHYS(pfn)     ((pfn) << PAGE_SHIFT)

#endif
