#ifndef MM_FRAME_ALLOC_H
#define MM_FRAME_ALLOC_H

#include <lib/bitmap.h>
#include <kernel/range.h>
#include <locking/spinlock.h>

typedef struct frame_s {
  struct frame_s* next;
  struct frame_s* prev;
  uint64_t pfn;
} frame_t;

typedef struct order_s {
  frame_t* free_list;
  size_t nr_free;
} order_t;

typedef struct section_s {
  range_t range;

  uint64_t base_pfn;
  size_t nr_frames;

  bitmap_t bitmap;

  order_t *orders;
  size_t nr_orders;

  raw_spinlock_t rlock;

  struct section_s* next;
} section_t;

typedef enum frame_alloc_err_e {
  FRAME_ALLOC_OK = 0,
  FRAME_ALLOC_INVALID
} frame_alloc_err_t;

extern frame_alloc_err_t frame_alloc_init(void);

#endif
