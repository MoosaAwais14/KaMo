#ifndef MM_PAGE_ALLOC_H
#define MM_PAGE_ALLOC_H

#include <stdint.h>
#include <stddef.h>

#include <kernel/address.h>

#include <asm/page_alloc.h>

#include <lib/bitmap.h>

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
  uint32_t flags;
} page_t;

typedef unsigned long pfn_t;

extern page_alloc_err_t page_alloc_init(void);

extern page_t* alloc_pages(zone_type_t zone_type, uint8_t order);
extern void free_pages(page_t* page, uint8_t order);

extern page_t* pfn_to_page(pfn_t pfn);
extern pfn_t page_to_pfn(page_t* page);

#define PHYS_PFN(phys)  (phys >> PAGE_SHIFT)
#define PFN_PHYS(pfn)   (pfn << PAGE_SHIFT)

#endif
