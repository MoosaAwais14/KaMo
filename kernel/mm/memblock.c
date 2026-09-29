#include <mm/memblock.h>

#include <asm/memblock.h>

#include <mm/earlybump.h>

#include <lib/tree/rb_tree.h>

static int memblock_rb_tree_cmp(const void* a, const void* b);

static rb_tree_node_t* memblock_find_overlap(const rb_tree_t* tree, const range_t* range);
static uint8_t memblock_find_free_in_range(const range_t* memory_range, size_t size, uintptr_t* result);
static memblock_err_t memblock_add_range(rb_tree_t* tree, range_t physical_range);

static rb_tree_t rb_tree_memory = { 0 };
static rb_tree_t rb_tree_reserved = { 0 };

memblock_err_t memblock_init(void)
{
  if (rb_tree_init(&rb_tree_memory, memblock_rb_tree_cmp))
    return MEMBLOCK_ERR_INVALID;
  if (rb_tree_init(&rb_tree_reserved, memblock_rb_tree_cmp))
    return MEMBLOCK_ERR_INVALID;

  return MEMBLOCK_OK;
}

memblock_err_t memblock_add(range_t physical_range)
{
  if (!rb_tree_memory.cmp || physical_range.end < physical_range.start || physical_range.end > UINTPTR_MAX)
    return MEMBLOCK_ERR_INVALID;

  return memblock_add_range(&rb_tree_memory, physical_range);
}

memblock_err_t memblock_reserve(range_t physical_range)
{
  if (!rb_tree_reserved.cmp || physical_range.end < physical_range.start || physical_range.end > UINTPTR_MAX)
    return MEMBLOCK_ERR_INVALID;

  return memblock_add_range(&rb_tree_reserved, physical_range);
}

void* memblock_alloc(size_t size)
{
  if (!rb_tree_memory.cmp || size == 0)
    return NULL;

  rb_tree_node_t* node = rb_tree_minimum(&rb_tree_memory, rb_tree_memory.root);
  while (node != rb_tree_memory.NIL)
  {
    range_t* memory = node->data;
    uintptr_t address;

    if (memory && memblock_find_free_in_range(memory, size, &address))
    {
      range_t allocation = {
        .start = address,
        .end = (uint64_t)address + (uint64_t)size - 1
      };

      if (memblock_reserve(allocation) != MEMBLOCK_OK)
        return NULL;

      return (void*)address;
    }

    node = rb_tree_successor(&rb_tree_memory, node);
  }

  return NULL;
}

uintptr_t memblock_start(void)
{
  if (!rb_tree_memory.cmp || rb_tree_memory.root == rb_tree_memory.NIL)
    return 0;

  rb_tree_node_t* node = rb_tree_minimum(&rb_tree_memory, rb_tree_memory.root);
  range_t* range = node->data;
  return range ? (uintptr_t)range->start : 0;
}

uintptr_t memblock_end(void)
{
  if (!rb_tree_memory.cmp || rb_tree_memory.root == rb_tree_memory.NIL)
    return 0;

  rb_tree_node_t* node = rb_tree_maximum(&rb_tree_memory, rb_tree_memory.root);
  range_t* range = node->data;
  return range ? (uintptr_t)range->end : 0;
}

static memblock_err_t memblock_add_range(rb_tree_t* tree, range_t physical_range)
{
  rb_tree_node_t* fnode = memblock_find_overlap(tree, &physical_range);
  if (!fnode)
  {
    rb_tree_node_t* node = earlybump_alloc(sizeof(*node), __alignof__(rb_tree_node_t));
    if (!node)
      return MEMBLOCK_ERR_EARLYBUMP;

    range_t* range = earlybump_alloc(sizeof(*range), __alignof__(range_t));
    if (!range)
      return MEMBLOCK_ERR_EARLYBUMP;

    *range = physical_range;
    node->data = range;
    return rb_tree_insert(tree, node) ? MEMBLOCK_ERR_INVALID : MEMBLOCK_OK;
  }

  range_t* frange = fnode->data;
  if (!frange)
    return MEMBLOCK_ERR_INVALID;
  if (range_contains(frange, &physical_range))
    return MEMBLOCK_OK;

  range_t merged_range = physical_range;
  rb_tree_node_t* reused_node = NULL;
  range_t* reused_range = NULL;

  while ((fnode = memblock_find_overlap(tree, &merged_range)))
  {
    frange = fnode->data;
    if (!frange)
      return MEMBLOCK_ERR_INVALID;

    if (!reused_node)
    {
      reused_node = fnode;
      reused_range = frange;
    }

    if (rb_tree_remove(tree, fnode))
      return MEMBLOCK_ERR_INVALID;

    if (frange->start < merged_range.start)
      merged_range.start = frange->start;
    if (frange->end > merged_range.end)
      merged_range.end = frange->end;
  }

  *reused_range = merged_range;
  return rb_tree_insert(tree, reused_node) ? MEMBLOCK_ERR_INVALID : MEMBLOCK_OK;
}

static rb_tree_node_t* memblock_find_overlap(const rb_tree_t* tree, const range_t* range)
{
  rb_tree_node_t* node = tree->root;

  while (node != tree->NIL)
  {
    range_t* current = node->data;
    if (current->end < range->start)
      node = node->right;
    else if (range->end < current->start)
      node = node->left;
    else
      return node;
  }

  return NULL;
}

static uint8_t memblock_find_free_in_range(const range_t* memory_range, size_t size, uintptr_t* result)
{
  if (!memory_range || !result || size == 0 || memory_range->start > UINTPTR_MAX)
    return 0;

  uint64_t limit = memory_range->end < UINTPTR_MAX ? memory_range->end : UINTPTR_MAX;
  uint64_t candidate = (memory_range->start + (MEMBLOCK_SIZE - 1)) & ~(MEMBLOCK_SIZE - 1);

  if (candidate < memory_range->start)
    return 0;

  while (candidate <= limit)
  {
    if ((uint64_t)size - 1 > limit - candidate)
      return 0;

    range_t requested = {
      .start = candidate,
      .end = candidate + (uint64_t)size - 1
    };

    rb_tree_node_t* overlap = memblock_find_overlap(&rb_tree_reserved, &requested);
    if (!overlap)
    {
      *result = (uintptr_t)candidate;
      return 1;
    }

    range_t* reserved = overlap->data;
    if (!reserved || reserved->end >= limit)
      return 0;

    candidate = (reserved->end + MEMBLOCK_SIZE) & ~(MEMBLOCK_SIZE - 1);
    if (candidate <= reserved->end)
      return 0;
  }

  return 0;
}

static int memblock_rb_tree_cmp(const void* a, const void* b)
{
  const range_t* range_a = a;
  const range_t* range_b = b;

  if (range_a->start < range_b->start)
    return -1;

  if (range_a->start > range_b->start)
    return 1;

  return 0;
}
