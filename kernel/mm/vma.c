#include <mm/vma.h>

#include <asm/vma.h>

#include <kernel/range.h>

#include <lib/memory.h>

struct vma_node_s {
  rb_tree_node_t rb_node;
  range_t range;

  vma_flags_t flags;

  const vma_alloc_ops_t* allocator;
};

static int vma_rb_tree_cmp(const void* a, const void* b);

static rb_tree_node_t* vma_find_overlap(const rb_tree_t* tree, const range_t* range);
static uint8_t vma_find_free_range_locked(const vma_space_t* space, virt_addr_t size, virt_addr_t aligned, range_t* result);

const vma_alloc_ops_t* vma_alloc_ops = NULL;

vma_err_t vma_set_vma_allocator(const vma_alloc_ops_t* vma_alloc)
{
  if (!vma_alloc || !vma_alloc->alloc || !vma_alloc->free)
    return VMA_ERR_BAD_ARGS;

  vma_alloc_ops = vma_alloc;
  return VMA_OK;
}

vma_err_t vma_init(vma_space_t* space, range_t bounds)
{
  if (!space || bounds.start > bounds.end ||
      !IS_ALIGNED(bounds.start, VMA_SIZE) ||
      (bounds.end & (VMA_SIZE - 1)) != VMA_SIZE - 1)
    return VMA_ERR_BAD_ARGS;

  space->rlock = RAW_SPINLOCK_UNLOCKED;

  if(rb_tree_init(&space->rb_tree, vma_rb_tree_cmp))
    return VMA_ERR_INVALID;

  space->bounds = bounds;

  return VMA_OK;
}

vma_err_t vma_reserve(vma_space_t* space, range_t range, vma_flags_t flags)
{
  if (!space || !space->rb_tree.cmp || !space->rb_tree.NIL ||
      range.start > range.end ||
      !IS_ALIGNED(range.start, VMA_SIZE) ||
      (range.end & (VMA_SIZE - 1)) != VMA_SIZE - 1 ||
      !range_contains(&space->bounds, &range))
    return VMA_ERR_BAD_ARGS;

  unsigned long irq_flags = raw_spin_lock_irqsave(&space->rlock);
  if (vma_find_overlap(&space->rb_tree, &range))
  {
    raw_spin_unlock_irqrestore(&space->rlock, irq_flags);
    return VMA_ERR_OVERLAP;
  }

  const vma_alloc_ops_t* allocator = vma_alloc_ops;
  raw_spin_unlock_irqrestore(&space->rlock, irq_flags);

  if (!allocator || !allocator->alloc || !allocator->free)
    return VMA_ERR_ALLOCATOR;

  virt_addr_t allocation = allocator->alloc(
    allocator->ctx, sizeof(struct vma_node_s), __alignof__(struct vma_node_s));
  if (!allocation)
    return VMA_ERR_ALLOCATOR;

  struct vma_node_s* vma_node = (struct vma_node_s*)(uintptr_t)allocation;
  vma_node->range = range;
  vma_node->flags = flags;
  vma_node->allocator = allocator;
  vma_node->rb_node.data = vma_node;

  irq_flags = raw_spin_lock_irqsave(&space->rlock);
  if (vma_find_overlap(&space->rb_tree, &range))
  {
    raw_spin_unlock_irqrestore(&space->rlock, irq_flags);
    allocator->free(allocator->ctx, allocation);
    return VMA_ERR_OVERLAP;
  }

  int insert_error = rb_tree_insert(&space->rb_tree, &vma_node->rb_node);
  raw_spin_unlock_irqrestore(&space->rlock, irq_flags);

  if (insert_error)
  {
    allocator->free(allocator->ctx, allocation);
    return VMA_ERR_INVALID;
  }

  return VMA_OK;
}

vma_err_t vma_alloc(vma_space_t* space, size_t size, size_t aligned, vma_flags_t flags, range_t* out_range)
{
  if (!space || !space->rb_tree.cmp || !space->rb_tree.NIL || !out_range ||
      size == 0)
    return VMA_ERR_BAD_ARGS;

  if (aligned == 0)
    aligned = VMA_SIZE;

  if (aligned < VMA_SIZE || (aligned & (aligned - 1)) != 0 ||
      size > UINTPTR_MAX - (VMA_SIZE - 1))
    return VMA_ERR_BAD_ARGS;

  virt_addr_t rounded_size = ALIGN_UP((virt_addr_t)size, VMA_SIZE);
  if (rounded_size == 0)
    return VMA_ERR_BAD_ARGS;

  const vma_alloc_ops_t* allocator = vma_alloc_ops;
  if (!allocator || !allocator->alloc || !allocator->free)
    return VMA_ERR_ALLOCATOR;

  for (;;)
  {
    range_t range;
    unsigned long irq_flags = raw_spin_lock_irqsave(&space->rlock);
    uint8_t found = vma_find_free_range_locked(
      space, rounded_size, aligned, &range);
    raw_spin_unlock_irqrestore(&space->rlock, irq_flags);

    if (!found)
      return VMA_ERR_NO_SPACE;

    virt_addr_t allocation = allocator->alloc(
      allocator->ctx, sizeof(struct vma_node_s), _Alignof(struct vma_node_s));
    if (!allocation)
      return VMA_ERR_ALLOCATOR;

    struct vma_node_s* vma_node = (struct vma_node_s*)(uintptr_t)allocation;
    vma_node->range = range;
    vma_node->flags = flags;
    vma_node->allocator = allocator;
    vma_node->rb_node.data = vma_node;

    irq_flags = raw_spin_lock_irqsave(&space->rlock);
    if (vma_find_overlap(&space->rb_tree, &range))
    {
      raw_spin_unlock_irqrestore(&space->rlock, irq_flags);
      allocator->free(allocator->ctx, allocation);
      continue;
    }

    int insert_error = rb_tree_insert(&space->rb_tree, &vma_node->rb_node);
    raw_spin_unlock_irqrestore(&space->rlock, irq_flags);

    if (insert_error)
    {
      allocator->free(allocator->ctx, allocation);
      return VMA_ERR_INVALID;
    }

    *out_range = range;
    return VMA_OK;
  }
}

vma_err_t vma_release(vma_space_t* space, range_t range)
{
  if (!space || !space->rb_tree.cmp || !space->rb_tree.NIL ||
      range.start > range.end)
    return VMA_ERR_BAD_ARGS;

  unsigned long irq_flags = raw_spin_lock_irqsave(&space->rlock);
  rb_tree_node_t* node = vma_find_overlap(&space->rb_tree, &range);
  if (!node)
  {
    raw_spin_unlock_irqrestore(&space->rlock, irq_flags);
    return VMA_ERR_NOT_FOUND;
  }

  struct vma_node_s* vma_node = node->data;
  if (!vma_node || vma_node->range.start != range.start || vma_node->range.end != range.end)
  {
    raw_spin_unlock_irqrestore(&space->rlock, irq_flags);
    return VMA_ERR_NOT_FOUND;
  }

  const vma_alloc_ops_t* allocator = vma_node->allocator;
  if (!allocator || !allocator->free || rb_tree_remove(&space->rb_tree, node))
  {
    raw_spin_unlock_irqrestore(&space->rlock, irq_flags);
    return VMA_ERR_INVALID;
  }

  raw_spin_unlock_irqrestore(&space->rlock, irq_flags);
  allocator->free(allocator->ctx, (virt_addr_t)(uintptr_t)vma_node);
  return VMA_OK;
}

static int vma_rb_tree_cmp(const void* a, const void* b)
{
  const struct vma_node_s* vma_a = a;
  const struct vma_node_s* vma_b = b;

  if (vma_a->range.start < vma_b->range.start)
    return -1;

  if (vma_a->range.start > vma_b->range.start)
    return 1;

  return 0;
}

static rb_tree_node_t* vma_find_overlap(const rb_tree_t* tree, const range_t* range)
{
  rb_tree_node_t* node = tree->root;

  while (node != tree->NIL)
  {
    struct vma_node_s* current = node->data;
    if (current->range.end < range->start)
      node = node->right;
    else if (range->end < current->range.start)
      node = node->left;
    else
      return node;
  }

  return NULL;
}

static uint8_t vma_find_free_range_locked(const vma_space_t* space, virt_addr_t size, virt_addr_t aligned, range_t* result)
{
  virt_addr_t candidate = ALIGN_UP(space->bounds.start, aligned);
  if (candidate < space->bounds.start)
    return 0;

  rb_tree_node_t* node = rb_tree_minimum(&space->rb_tree, space->rb_tree.root);

  while (node != space->rb_tree.NIL)
  {
    struct vma_node_s* current = node->data;

    if (candidate < current->range.start && current->range.start - candidate >= size)
    {
      result->start = candidate;
      result->end = candidate + size - 1;
      return 1;
    }

    if (candidate <= current->range.end)
    {
      if (current->range.end == UINTPTR_MAX)
        return 0;

      virt_addr_t next = current->range.end + 1;
      candidate = ALIGN_UP(next, aligned);
      if (candidate < next)
        return 0;
    }

    node = rb_tree_successor(&space->rb_tree, node);
    if (!node)
      break;
  }

  if (candidate > space->bounds.end ||
      size - 1 > space->bounds.end - candidate)
    return 0;

  result->start = candidate;
  result->end = candidate + size - 1;
  return 1;
}
