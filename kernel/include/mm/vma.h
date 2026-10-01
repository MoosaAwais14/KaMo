#ifndef MM_VMA_H
#define MM_VMA_H

#include <stdint.h>
#include <stddef.h>

#include <kernel/address.h>
#include <kernel/range.h>
#include <locking/spinlock.h>
#include <lib/tree/rb_tree.h>

typedef enum vma_err_s {
  VMA_OK,
  VMA_ERR_BAD_ARGS,
  VMA_ERR_INVALID,
  VMA_ERR_OVERLAP,
  VMA_ERR_NOT_FOUND,
  VMA_ERR_ALLOCATOR,
  VMA_ERR_NO_SPACE
} vma_err_t;

typedef struct vma_alloc_ops_s {
  virt_addr_t (*alloc)(void* ctx, size_t size, size_t aligned);
  void (*free)(void* ctx, virt_addr_t vaddr);
  void* ctx;
} vma_alloc_ops_t;

typedef struct vma_space_s {
  rb_tree_t rb_tree;
  range_t bounds;
  raw_spinlock_t rlock;
} vma_space_t;

typedef uint64_t vma_flags_t;

extern vma_err_t vma_set_vma_allocator(const vma_alloc_ops_t* vma_alloc);

extern vma_err_t vma_init(vma_space_t* space, range_t bounds);

extern vma_err_t vma_reserve(vma_space_t* space, range_t range, vma_flags_t flags);
extern vma_err_t vma_alloc(vma_space_t* space, size_t size, size_t aligned, vma_flags_t flags, range_t* out_range);
extern vma_err_t vma_release(vma_space_t* space, range_t range);


extern const vma_alloc_ops_t* vma_alloc_ops;

#endif
