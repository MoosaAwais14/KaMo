#ifndef MM_VMA_H
#define MM_VMA_H

#include <stdint.h>
#include <stddef.h>

#include <kernel/range.h>
#include <locking/spinlock.h>
#include <lib/tree/rb_tree.h>

#include <mm/page_alloc.h>

typedef struct vma_space_s {
  rb_tree_t busy_tree;
  range_t limits;
  raw_spinlock_t rlock;
} vma_space_t;

typedef struct vma_node_s {
  range_t range;
  page_t* pages;
} vma_node_t;

extern void vma_init(vma_space_t* vma_space, range_t virtual_limits);

#endif
