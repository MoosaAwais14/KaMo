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
static vma_err_t vma_add_range(vma_space_t* space, range_t new_range, vma_flags_t flags);

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

  const vma_alloc_ops_t* allocator = vma_alloc_ops;
  if (!allocator || !allocator->alloc || !allocator->free)
    return VMA_ERR_ALLOCATOR;

  unsigned long irq_flags = raw_spin_lock_irqsave(&space->rlock);
  vma_err_t err = vma_add_range(space, range, flags);
  raw_spin_unlock_irqrestore(&space->rlock, irq_flags);

  return err;
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

    irq_flags = raw_spin_lock_irqsave(&space->rlock);
    vma_err_t err = vma_add_range(space, range, flags);
    raw_spin_unlock_irqrestore(&space->rlock, irq_flags);

    if (err == VMA_ERR_OVERLAP)
    {
      continue;
    }
    else if (err != VMA_OK)
    {
      return err;
    }

    *out_range = range;
    return VMA_OK;
  }
}

vma_err_t vma_alloc_range(vma_space_t* space, size_t size, size_t aligned, vma_flags_t flags, range_t range, range_t* out_range)
{
  if (!space || !space->rb_tree.cmp || !space->rb_tree.NIL || !out_range ||
    size == 0 || range.start > range.end ||
    !IS_ALIGNED(range.start, VMA_SIZE) ||
    (range.end & (VMA_SIZE - 1)) != VMA_SIZE - 1 ||
    !range_contains(&space->bounds, &range))
    return VMA_ERR_BAD_ARGS;

  if (aligned == 0)
    aligned = VMA_SIZE;

  if (aligned < VMA_SIZE || (aligned & (aligned - 1)) != 0 ||
    size > UINTPTR_MAX - (VMA_SIZE - 1))
    return VMA_ERR_BAD_ARGS;

  virt_addr_t rounded_size = ALIGN_UP((virt_addr_t)size, VMA_SIZE);
  if (rounded_size == 0 || rounded_size > range.end - range.start + 1)
    return VMA_ERR_BAD_ARGS;

  virt_addr_t candidate = ALIGN_UP(range.start, aligned);
  if (candidate < range.start ||
    candidate > range.end ||
    rounded_size - 1 > range.end - candidate)
    return VMA_ERR_NO_SPACE;

  range_t allocation_range = {
    .start = candidate,
    .end = candidate + rounded_size - 1
  };

  unsigned long irq_flags = raw_spin_lock_irqsave(&space->rlock);
  vma_err_t err = vma_add_range(space, allocation_range, flags);
  raw_spin_unlock_irqrestore(&space->rlock, irq_flags);

  if (err != VMA_OK)
    return err;

  *out_range = allocation_range;
  return VMA_OK;
}

vma_err_t vma_release(vma_space_t* space, range_t range)
{
  if (!space || !space->rb_tree.cmp || !space->rb_tree.NIL ||
      range.start > range.end)
    return VMA_ERR_BAD_ARGS;

  const vma_alloc_ops_t* allocator = vma_alloc_ops;
  if (!allocator || !allocator->free)
    return VMA_ERR_ALLOCATOR;

  unsigned long irq_flags = raw_spin_lock_irqsave(&space->rlock);
  rb_tree_node_t* node = vma_find_overlap(&space->rb_tree, &range);
  if (!node)
  {
    raw_spin_unlock_irqrestore(&space->rlock, irq_flags);
    return VMA_ERR_NOT_FOUND;
  }

  struct vma_node_s* vma_node = node->data;
  if (!vma_node || !range_contains(&vma_node->range, &range))
  {
    raw_spin_unlock_irqrestore(&space->rlock, irq_flags);
    return VMA_ERR_NOT_FOUND;
  }

  if (range.start == vma_node->range.start && range.end == vma_node->range.end)
  {
    if (rb_tree_remove(&space->rb_tree, node))
    {
      raw_spin_unlock_irqrestore(&space->rlock, irq_flags);
      return VMA_ERR_INVALID;
    }

    raw_spin_unlock_irqrestore(&space->rlock, irq_flags);
    allocator->free(allocator->ctx, (virt_addr_t)(uintptr_t)vma_node);
    return VMA_OK;
  }

  if (range.start == vma_node->range.start)
  {
    if (range.end == UINTPTR_MAX)
    {
      raw_spin_unlock_irqrestore(&space->rlock, irq_flags);
      return VMA_ERR_INVALID;
    }

    if (rb_tree_remove(&space->rb_tree, node))
    {
      raw_spin_unlock_irqrestore(&space->rlock, irq_flags);
      return VMA_ERR_INVALID;
    }

    vma_node->range.start = range.end + 1;

    if (rb_tree_insert(&space->rb_tree, node))
    {
      raw_spin_unlock_irqrestore(&space->rlock, irq_flags);
      return VMA_ERR_INVALID;
    }

    raw_spin_unlock_irqrestore(&space->rlock, irq_flags);
    return VMA_OK;
  }

  if (range.end == vma_node->range.end)
  {
    vma_node->range.end = range.start - 1;
    raw_spin_unlock_irqrestore(&space->rlock, irq_flags);
    return VMA_OK;
  }

  if (range.end == UINTPTR_MAX)
  {
    raw_spin_unlock_irqrestore(&space->rlock, irq_flags);
    return VMA_ERR_INVALID;
  }

  raw_spin_unlock_irqrestore(&space->rlock, irq_flags);

  if (!allocator->alloc)
    return VMA_ERR_ALLOCATOR;

  virt_addr_t right_allocation = allocator->alloc(
    allocator->ctx, sizeof(struct vma_node_s), _Alignof(struct vma_node_s));
  if (!right_allocation)
    return VMA_ERR_ALLOCATOR;

  struct vma_node_s* right_node_data = (struct vma_node_s*)(uintptr_t)right_allocation;
  
  irq_flags = raw_spin_lock_irqsave(&space->rlock);

  virt_addr_t original_end = vma_node->range.end;

  if (rb_tree_remove(&space->rb_tree, node))
  {
    raw_spin_unlock_irqrestore(&space->rlock, irq_flags);
    allocator->free(allocator->ctx, right_allocation);
    return VMA_ERR_INVALID;
  }

  right_node_data->range.start = range.end + 1;
  right_node_data->range.end = original_end;
  right_node_data->flags = vma_node->flags;
  right_node_data->allocator = allocator;
  right_node_data->rb_node.data = right_node_data;

  vma_node->range.end = range.start - 1;

  if (rb_tree_insert(&space->rb_tree, node) ||
      rb_tree_insert(&space->rb_tree, &right_node_data->rb_node))
  {
    raw_spin_unlock_irqrestore(&space->rlock, irq_flags);
    return VMA_ERR_INVALID;
  }

  raw_spin_unlock_irqrestore(&space->rlock, irq_flags);
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

static vma_err_t vma_add_range(vma_space_t* space, range_t new_range, vma_flags_t flags)
{
  const vma_alloc_ops_t* allocator = vma_alloc_ops;
  range_t merged_range = new_range;

  range_t search_range = {
    .start = (merged_range.start == 0) ? 0 : merged_range.start - 1,
    .end = (merged_range.end == UINTPTR_MAX) ? UINTPTR_MAX : merged_range.end + 1
  };

  rb_tree_node_t* fnode = vma_find_overlap(&space->rb_tree, &search_range);

  if (!fnode)
  {
    virt_addr_t allocation = allocator->alloc(
      allocator->ctx, sizeof(struct vma_node_s), _Alignof(struct vma_node_s));
    if (!allocation)
      return VMA_ERR_ALLOCATOR;

    struct vma_node_s* vma_node = (struct vma_node_s*)(uintptr_t)allocation;
    vma_node->range = new_range;
    vma_node->flags = flags;
    vma_node->allocator = allocator;
    vma_node->rb_node.data = vma_node;

    return rb_tree_insert(&space->rb_tree, &vma_node->rb_node) ? VMA_ERR_INVALID : VMA_OK;
  }

  rb_tree_node_t* reused_node = NULL;
  struct vma_node_s* reused_vma = NULL;

  while ((fnode = vma_find_overlap(&space->rb_tree, &search_range)))
  {
    struct vma_node_s* current = fnode->data;
    if (!current)
      return VMA_ERR_INVALID;

    if (!reused_node)
    {
      reused_node = fnode;
      reused_vma = current;
    }
    else
  {
      if (rb_tree_remove(&space->rb_tree, fnode))
        return VMA_ERR_INVALID;
      allocator->free(allocator->ctx, (virt_addr_t)(uintptr_t)current);
    }

    if (current->range.start < merged_range.start)
      merged_range.start = current->range.start;
    if (current->range.end > merged_range.end)
      merged_range.end = current->range.end;

    if (fnode == reused_node)
    {
      if (rb_tree_remove(&space->rb_tree, reused_node))
        return VMA_ERR_INVALID;
    }

    search_range.start = (merged_range.start == 0) ? 0 : merged_range.start - 1;
    search_range.end = (merged_range.end == UINTPTR_MAX) ? UINTPTR_MAX : merged_range.end + 1;
  }

  reused_vma->range = merged_range;
  reused_vma->flags = flags;

  return rb_tree_insert(&space->rb_tree, reused_node) ? VMA_ERR_INVALID : VMA_OK;
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
