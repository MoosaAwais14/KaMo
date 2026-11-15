#include <mm/vma.h>
#include <asm/vma.h>
#include <kernel/range.h>
#include <lib/memory.h>
#include <mm/slab.h>

static int vma_rb_tree_cmp(const void* a, const void* b);

static rb_tree_node_t* vma_first(vma_space_t* vma_space, rb_tree_node_t** iter, range_t* out);
static rb_tree_node_t* vma_next(vma_space_t* vma_space, rb_tree_node_t** iter, range_t* out);

static rb_tree_node_t* vma_find_overlap(const rb_tree_t* tree, const range_t* range);
static uint8_t vma_find_free_in_range(const rb_tree_t* tree, const range_t* memory_range, size_t size, size_t aligned, virt_addr_t* result);

static rb_tree_node_t* vma_add(rb_tree_t* tree, range_t range, uint32_t flags);
static int vma_remove(rb_tree_t* tree, rb_tree_node_t* rb_node);

void vma_init(vma_space_t* vma_space, range_t virtual_limits)
{
  if(!vma_space)
    return;

  rb_tree_init(&vma_space->busy_tree, vma_rb_tree_cmp);
  vma_space->rlock = RAW_SPINLOCK_UNLOCKED;
  vma_space->limits = virtual_limits;
}

vma_node_t* vma_alloc_region(vma_space_t* vma_space, size_t size, size_t aligned, range_t range, uint32_t flags)
{
  if(!vma_space || size == 0)
    return NULL;

  if (aligned == 0)
    aligned = VMA_SIZE;

  if ((aligned & (aligned - 1)) != 0)
    return NULL;

  if (range.end < range.start)
    return NULL;

  rb_tree_node_t* iter;
  range_t memory_range;

  unsigned long irq_flags = raw_spin_lock_irqsave(&vma_space->rlock);

  if(!vma_first(vma_space, &iter, &memory_range))
  {
    memory_range = vma_space->limits;
    iter = NULL;
  }

  do
  {
    virt_addr_t sub_start = (memory_range.start > range.start) ? memory_range.start : range.start;
    virt_addr_t sub_end = (memory_range.end < range.end) ? memory_range.end : range.end;

    if (sub_start <= sub_end)
    {
      range_t effective_range = {
        .start = sub_start,
        .end = sub_end
      };

      virt_addr_t start;

      if (vma_find_free_in_range(&vma_space->busy_tree, &effective_range, size, aligned, &start))
      {
        if ((virt_addr_t)(size - 1) > UINTPTR_MAX - start)
        {
          raw_spin_unlock_irqrestore(&vma_space->rlock, irq_flags);
          return NULL;
        }

        range_t allocation = {
          .start = start,
          .end = start + (virt_addr_t)(size - 1)
        };

        rb_tree_node_t* rb_node = vma_add(&vma_space->busy_tree, allocation, flags);
        raw_spin_unlock_irqrestore(&vma_space->rlock, irq_flags);
        return rb_node ? rb_node->data : NULL;
      }
    }

    if (!iter)
      break;

  } while(vma_next(vma_space, &iter, &memory_range));

  raw_spin_unlock_irqrestore(&vma_space->rlock, irq_flags);
  return NULL;
}

int vma_free_region(vma_space_t* vma_space, vma_node_t* vma_node)
{
  if(!vma_space || !vma_node)
    return -1;

  range_t requested = vma_node->range;

  unsigned long irq_flags = raw_spin_lock_irqsave(&vma_space->rlock);
  int ret = vma_remove(&vma_space->busy_tree, vma_node->rb_owner);
  raw_spin_unlock_irqrestore(&vma_space->rlock, irq_flags);
  return ret;
}

vma_node_t* vma_find(vma_space_t* vma_space, virt_addr_t address)
{
  if (!vma_space)
    return NULL;

  range_t requested = {
    .start = address,
    .end = address
  };

  unsigned long irq_flags = raw_spin_lock_irqsave(&vma_space->rlock);

  if(vma_space->limits.start > address || vma_space->limits.end < address)
  {
    raw_spin_unlock_irqrestore(&vma_space->rlock, irq_flags);
    return NULL;
  }

  rb_tree_node_t* overlap = vma_find_overlap(&vma_space->busy_tree, &requested);

  raw_spin_unlock_irqrestore(&vma_space->rlock, irq_flags);
  return overlap ? ((vma_node_t*)overlap->data) : NULL;
}

static rb_tree_node_t* vma_add(rb_tree_t* tree, range_t range, uint32_t flags)
{
  if(!tree)
    return NULL;

  rb_tree_node_t* rb_node = kmalloc(sizeof(rb_tree_node_t));
  if(!rb_node)
    return NULL;

  vma_node_t* vma_node = kmalloc(sizeof(vma_node_t));
  if(!vma_node)
  {
    kfree(rb_node);
    return NULL;
  }

  vma_node->range = range;
  vma_node->flags = flags;
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

static rb_tree_node_t* vma_first(vma_space_t* vma_space, rb_tree_node_t** iter, range_t* out)
{
  if(!vma_space || !vma_space->busy_tree.root || !iter)
    return NULL;

  rb_tree_node_t* rb_node = rb_tree_minimum(&vma_space->busy_tree, vma_space->busy_tree.root);

  if(rb_node == vma_space->busy_tree.NIL)
    return NULL;

  vma_node_t* vma_node = rb_node->data;

  if(!vma_node)
    return NULL;

  if(out)
    *out = vma_node->range;

  *iter = rb_node;
  return rb_node;
}

static rb_tree_node_t* vma_next(vma_space_t* vma_space, rb_tree_node_t** iter, range_t* out)
{
  if(!vma_space || !vma_space->busy_tree.root || !iter)
    return NULL;

  rb_tree_node_t* rb_node = rb_tree_successor(&vma_space->busy_tree, *iter);

  if(rb_node == vma_space->busy_tree.NIL)
    return NULL;

  vma_node_t* vma_node = rb_node->data;

  if(!vma_node)
    return NULL;

  if(out)
    *out = vma_node->range;

  *iter = rb_node;
  return rb_node;
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
