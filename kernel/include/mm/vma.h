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

typedef uint32_t vma_flags_t;

#define VMA_FLAG_VMALLOC   (1u << 0)
#define VMA_FLAG_VMAP      (1u << 1)
#define VMA_FLAG_FREEING   (1u << 2)

typedef struct vma_node_s {
  range_t range;
  vma_flags_t flags;
  page_t** pages;
  size_t nr_pages;
  page_t* page_array_storage;
  uint8_t page_array_order;
  rb_tree_node_t* rb_owner;
} vma_node_t;

extern void vma_init(vma_space_t* vma_space, range_t virtual_limits);

extern vma_node_t* vma_alloc_region(vma_space_t* vma_space, size_t size, size_t aligned, range_t range);
extern int vma_set_mapping(vma_space_t* vma_space, vma_node_t* vma_node, page_t** pages, size_t nr_pages, vma_flags_t flags, page_t* page_array_storage, uint8_t page_array_order);
extern vma_node_t* vma_claim_region(vma_space_t* vma_space, virt_addr_t address, vma_flags_t required_flags);
extern int vma_free_claimed_region(vma_space_t* vma_space, vma_node_t* vma_node);
extern int vma_free_region(vma_space_t* vma_space, vma_node_t* vma_node);

#endif
