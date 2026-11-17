#include <mm/vma.h>
#include <asm/vma.h>
#include <kernel/range.h>
#include <lib/memory.h>
#include <mm/slab.h>

static int vma_rb_tree_cmp(const void* a, const void* b);

static rb_tree_node_t* vma_find_overlap(const rb_tree_t* tree, const range_t* range);
static uint8_t vma_find_free_in_range(const rb_tree_t* tree, const range_t* memory_range, size_t size, size_t aligned, virt_addr_t* result);

static rb_tree_node_t* vma_add(rb_tree_t* tree, range_t range);
static int vma_remove(rb_tree_t* tree, rb_tree_node_t* rb_node);

void vma_init(vma_space_t* vma_space, range_t virtual_limits)
{
  if(!vma_space)
    return;

  rb_tree_init(&vma_space->busy_tree, vma_rb_tree_cmp);
  vma_space->rlock = RAW_SPINLOCK_UNLOCKED;
  vma_space->limits = virtual_limits;
}

vma_node_t* vma_alloc_region(vma_space_t* vma_space, size_t size, size_t aligned, range_t range)
{
  if(!vma_space || size == 0)
    return NULL;

  if (aligned == 0)
    aligned = VMA_SIZE;

  if ((aligned & (aligned - 1)) != 0)
    return NULL;

  if (range.end < range.start)
    return NULL;

  range_t effective_range = {
    .start = (range.start > vma_space->limits.start) ? range.start : vma_space->limits.start,
    .end = (range.end < vma_space->limits.end) ? range.end : vma_space->limits.end
  };

  unsigned long irq_flags = raw_spin_lock_irqsave(&vma_space->rlock);

  if (effective_range.start <= effective_range.end)
  {
    virt_addr_t start;
    if (vma_find_free_in_range(&vma_space->busy_tree, &effective_range, size, aligned, &start))
    {
      if ((virt_addr_t)(size - 1) <= UINTPTR_MAX - start)
      {
        range_t allocation = {
          .start = start,
          .end = start + (virt_addr_t)(size - 1)
        };

        rb_tree_node_t* rb_node = vma_add(&vma_space->busy_tree, allocation);
        raw_spin_unlock_irqrestore(&vma_space->rlock, irq_flags);
        return rb_node ? rb_node->data : NULL;
      }
    }
  }

  raw_spin_unlock_irqrestore(&vma_space->rlock, irq_flags);
  return NULL;
}

int vma_free_region(vma_space_t* vma_space, vma_node_t* vma_node)
{
  if(!vma_space || !vma_node)
    return -1;

  unsigned long irq_flags = raw_spin_lock_irqsave(&vma_space->rlock);
  int ret = !(vma_node->flags & (VMA_FLAG_VMAP | VMA_FLAG_IOMAP | VMA_FLAG_FREEING)) &&
            vma_node->rb_owner && vma_node->rb_owner->data == vma_node
    ? vma_remove(&vma_space->busy_tree, vma_node->rb_owner)
    : -1;
  raw_spin_unlock_irqrestore(&vma_space->rlock, irq_flags);
  return ret;
}

int vma_free_claimed_region(vma_space_t* vma_space, vma_node_t* vma_node)
{
  if (!vma_space || !vma_node)
    return -1;

  unsigned long irq_flags = raw_spin_lock_irqsave(&vma_space->rlock);
  int ret = (vma_node->flags & VMA_FLAG_FREEING) &&
            (vma_node->flags & (VMA_FLAG_VMAP | VMA_FLAG_IOMAP)) &&
            vma_node->rb_owner && vma_node->rb_owner->data == vma_node
    ? vma_remove(&vma_space->busy_tree, vma_node->rb_owner)
    : -1;
  raw_spin_unlock_irqrestore(&vma_space->rlock, irq_flags);
  return ret;
}

int vma_set_page_backing(vma_space_t* vma_space, vma_node_t* vma_node, page_t** pages, size_t nr_pages, vma_flags_t flags, page_t* page_array_storage, uint8_t page_array_order)
{
  if (!vma_space || !vma_node || !pages || nr_pages == 0 || !(flags & VMA_FLAG_VMAP))
    return -1;

  if ((flags & VMA_FLAG_VMALLOC) && (!page_array_storage || page_array_order > MAX_PAGE_ORDER))
    return -1;

  unsigned long irq_flags = raw_spin_lock_irqsave(&vma_space->rlock);
  if (!vma_node->rb_owner || vma_node->rb_owner->data != vma_node ||
      vma_node->flags != 0)
  {
    raw_spin_unlock_irqrestore(&vma_space->rlock, irq_flags);
    return -1;
  }

  vma_node->pages = pages;
  vma_node->nr_pages = nr_pages;
  vma_node->page_array_storage = page_array_storage;
  vma_node->page_array_order = page_array_order;
  vma_node->flags = flags & ~VMA_FLAG_FREEING;

  raw_spin_unlock_irqrestore(&vma_space->rlock, irq_flags);
  return 0;
}

const vma_node_t* vma_find_region(vma_space_t* vma_space, virt_addr_t address)
{
  if (!vma_space)
    return NULL;

  range_t requested = {
    .start = address,
    .end = address
  };

  unsigned long irq_flags = raw_spin_lock_irqsave(&vma_space->rlock);
  rb_tree_node_t* overlap = NULL;

  if (address >= vma_space->limits.start && address <= vma_space->limits.end)
    overlap = vma_find_overlap(&vma_space->busy_tree, &requested);

  const vma_node_t* vma_node = overlap ? overlap->data : NULL;
  raw_spin_unlock_irqrestore(&vma_space->rlock, irq_flags);
  return vma_node;
}

int vma_set_iomap(vma_space_t* vma_space, vma_node_t* vma_node)
{
  if (!vma_space || !vma_node)
    return -1;

  unsigned long irq_flags = raw_spin_lock_irqsave(&vma_space->rlock);
  if (!vma_node->rb_owner || vma_node->rb_owner->data != vma_node ||
      vma_node->flags != 0)
  {
    raw_spin_unlock_irqrestore(&vma_space->rlock, irq_flags);
    return -1;
  }

  vma_node->flags = VMA_FLAG_IOMAP;
  raw_spin_unlock_irqrestore(&vma_space->rlock, irq_flags);
  return 0;
}

vma_node_t* vma_claim_region(vma_space_t* vma_space, virt_addr_t address, vma_flags_t required_flags)
{
  if (!vma_space)
    return NULL;

  range_t requested = {
    .start = address,
    .end = address
  };

  unsigned long irq_flags = raw_spin_lock_irqsave(&vma_space->rlock);
  rb_tree_node_t* overlap = NULL;

  if (address >= vma_space->limits.start && address <= vma_space->limits.end)
    overlap = vma_find_overlap(&vma_space->busy_tree, &requested);

  vma_node_t* vma_node = overlap ? (vma_node_t*)overlap->data : NULL;
  if (!vma_node || vma_node->range.start != address ||
      (vma_node->flags & required_flags) != required_flags ||
      (vma_node->flags & VMA_FLAG_FREEING))
  {
    raw_spin_unlock_irqrestore(&vma_space->rlock, irq_flags);
    return NULL;
  }

  vma_node->flags |= VMA_FLAG_FREEING;
  raw_spin_unlock_irqrestore(&vma_space->rlock, irq_flags);
  return vma_node;
}

static rb_tree_node_t* vma_add(rb_tree_t* tree, range_t range)
{
  if(!tree)
    return NULL;

  rb_tree_node_t* rb_node = kmalloc(sizeof(rb_tree_node_t));
  if(!rb_node)
    return NULL;

  memset(rb_node, 0, sizeof(rb_tree_node_t));

  vma_node_t* vma_node = kmalloc(sizeof(vma_node_t));
  if(!vma_node)
  {
    kfree(rb_node);
    return NULL;
  }
  memset(vma_node, 0, sizeof(vma_node_t));

  vma_node->range = range;
  vma_node->rb_owner = rb_node;
  rb_node->data = vma_node;

  if(rb_tree_insert(tree, rb_node))
  {
    kfree(vma_node);
    kfree(rb_node);
    return NULL;
  }

  return rb_node;
}

static int vma_remove(rb_tree_t* tree, rb_tree_node_t* rb_node)
{
  if (!tree || !rb_node)
    return -1;

  if(rb_tree_remove(tree, rb_node))
    return 1;

  kfree(rb_node->data);
  kfree(rb_node);

  return 0;
}

static rb_tree_node_t* vma_find_overlap(const rb_tree_t* tree, const range_t* range)
{
  if (!tree || !tree->root || !tree->NIL || !range)
    return NULL;

  rb_tree_node_t* node = tree->root;

  while (node != tree->NIL)
  {
    vma_node_t* current = node->data;
    if (current->range.end < range->start)
      node = node->right;
    else if (range->end < current->range.start)
      node = node->left;
    else
      return node;
  }

  return NULL;
}

static uint8_t vma_find_free_in_range(const rb_tree_t* tree, const range_t* memory_range, size_t size, size_t aligned, virt_addr_t* result)
{
  if (!memory_range || !result || size == 0)
    return 0;

  if (memory_range->start > memory_range->end)
    return 0;

  if (aligned == 0)
    aligned = VMA_SIZE;

  if ((aligned & (aligned - 1)) != 0)
    return 0;

  virt_addr_t limit = memory_range->end;

  virt_addr_t candidate = ALIGN_UP(memory_range->start, aligned);

  if (candidate < memory_range->start)
    return 0;

  while (candidate <= limit)
  {
    if ((virt_addr_t)(size - 1) > limit - candidate)
      return 0;

    range_t requested = {
      .start = candidate,
      .end = candidate + (virt_addr_t)(size - 1)
    };

    rb_tree_node_t* overlap = vma_find_overlap(tree, &requested);

    if (!overlap)
    {
      *result = candidate;
      return 1;
    }

    struct vma_node_s* reserved = overlap->data;

    if (!reserved)
      return 0;

    if (reserved->range.end >= limit)
      return 0;

    if (reserved->range.end == UINTPTR_MAX)
      return 0;

    virt_addr_t next = reserved->range.end + 1;

    candidate = ALIGN_UP(next, aligned);

    if (candidate < next)
      return 0;
  }

  return 0;
}

static int vma_rb_tree_cmp(const void* a, const void* b)
{
  const vma_node_t* node_a = a;
  const vma_node_t* node_b = b;

  if(node_a->range.start < node_b->range.start)
    return -1;

  if(node_a->range.start > node_b->range.start)
    return 1;

  return 0;
}
