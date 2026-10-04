#include <mm/memblock.h>

#include <asm/memblock.h>

#include <mm/earlybump.h>

#include <lib/tree/rb_tree.h>
#include <lib/memory.h>

static int memblock_rb_tree_cmp(const void* a, const void* b);

static rb_tree_node_t* memblock_find_overlap(const rb_tree_t* tree, const range_t* range);
static uint8_t memblock_find_free_in_range(const range_t* memory_range, size_t size, size_t aligned, phys_addr_t* result);

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

memblock_err_t memblock_alloc(size_t size, size_t aligned, range_t* out)
{
  if (!out || size == 0)
    return MEMBLOCK_ERR_INVALID;

  if (aligned == 0)
    aligned = MEMBLOCK_SIZE;

  if ((aligned & (aligned - 1)) != 0)
    return MEMBLOCK_ERR_INVALID;

  memblock_iter_t iter;
  range_t memory_range;

  if (memblock_memory_first(&iter, &memory_range))
    return MEMBLOCK_ERR_NOMEM;

  do
  {
    phys_addr_t start;

    if (!memblock_find_free_in_range(&memory_range, size, aligned, &start))
    {
      continue;
    }

    if ((phys_addr_t)(size - 1) > UINTPTR_MAX - start)
      return MEMBLOCK_ERR_NOMEM;

    range_t allocation = {
      .start = start,
      .end = start + (phys_addr_t)(size - 1)
    };

    memblock_err_t err = memblock_reserve(allocation);

    if (err != MEMBLOCK_OK)
      return err;

    *out = allocation;
    return MEMBLOCK_OK;

  } while (!memblock_memory_next(&iter, &memory_range));

  return MEMBLOCK_ERR_NOMEM;
}

memblock_err_t memblock_alloc_free(range_t range)
{
  if (!rb_tree_memory.cmp || !rb_tree_reserved.cmp)
    return MEMBLOCK_ERR_INVALID;

  if (range.end < range.start)
    return MEMBLOCK_ERR_INVALID;

  rb_tree_node_t* memory_node = memblock_find_overlap(&rb_tree_memory, &range);

  if (!memory_node)
    return MEMBLOCK_ERR_INVALID;

  range_t* memory_range = memory_node->data;

  if (!memory_range || !range_contains(memory_range, &range))
    return MEMBLOCK_ERR_INVALID;

  rb_tree_node_t* reserved_node = memblock_find_overlap(&rb_tree_reserved, &range);

  if (!reserved_node)
    return MEMBLOCK_ERR_INVALID;

  range_t* reserved_range = reserved_node->data;

  if (!reserved_range || !range_contains(reserved_range, &range))
    return MEMBLOCK_ERR_INVALID;

  if (range.start == reserved_range->start && range.end == reserved_range->end)
  {
    if (rb_tree_remove(&rb_tree_reserved, reserved_node))
      return MEMBLOCK_ERR_INVALID;

    return MEMBLOCK_OK;
  }

  if (range.start == reserved_range->start)
  {
    if (range.end == UINTPTR_MAX)
      return MEMBLOCK_ERR_INVALID;

    reserved_range->start = range.end + 1;

    if (rb_tree_remove(&rb_tree_reserved, reserved_node))
      return MEMBLOCK_ERR_INVALID;

    if (rb_tree_insert(&rb_tree_reserved, reserved_node))
      return MEMBLOCK_ERR_INVALID;

    return MEMBLOCK_OK;
  }

  if (range.end == reserved_range->end)
  {
    reserved_range->end = range.start - 1;
    return MEMBLOCK_OK;
  }

  if (range.end == UINTPTR_MAX)
    return MEMBLOCK_ERR_INVALID;

  rb_tree_node_t* right_node = earlybump_alloc(sizeof(*right_node), _Alignof(rb_tree_node_t));

  if (!right_node)
    return MEMBLOCK_ERR_EARLYBUMP;

  range_t* right_range = earlybump_alloc(sizeof(*right_range), _Alignof(range_t));

  if (!right_range)
    return MEMBLOCK_ERR_EARLYBUMP;

  right_range->start = range.end + 1;
  right_range->end = reserved_range->end;

  right_node->data = right_range;

  reserved_range->end = range.start - 1;

  if (rb_tree_insert(&rb_tree_reserved, right_node))
    return MEMBLOCK_ERR_INVALID;

  return MEMBLOCK_OK;
}

memblock_err_t memblock_memory_first(memblock_iter_t *iter, range_t *out)
{
  if (!iter || !out || !rb_tree_memory.root)
    return MEMBLOCK_ERR_INVALID;

  iter->node = rb_tree_minimum(&rb_tree_memory, rb_tree_memory.root);

  if (iter->node == rb_tree_memory.NIL)
    return MEMBLOCK_ERR_INVALID;

  *out = *(range_t*)((rb_tree_node_t*)iter->node)->data;
  return MEMBLOCK_OK;
}

memblock_err_t memblock_memory_next(memblock_iter_t *iter, range_t *out)
{
  if (!iter || !out || !rb_tree_memory.root)
    return MEMBLOCK_ERR_INVALID;

  if (!iter->node)
    return MEMBLOCK_ERR_INVALID;

  iter->node = rb_tree_successor(&rb_tree_memory, iter->node);

  if (!iter->node || iter->node == rb_tree_memory.NIL)
    return MEMBLOCK_ERR_INVALID;

  *out = *(range_t*)((rb_tree_node_t*)iter->node)->data;
  return MEMBLOCK_OK;
}

memblock_err_t memblock_reserved_first(memblock_iter_t *iter, range_t *out)
{
  if (!iter || !out || !rb_tree_reserved.root)
    return MEMBLOCK_ERR_INVALID;

  iter->node = rb_tree_minimum(&rb_tree_reserved, rb_tree_reserved.root);

  if (iter->node == rb_tree_reserved.NIL)
    return MEMBLOCK_ERR_INVALID;

  *out = *(range_t*)((rb_tree_node_t*)iter->node)->data;
  return MEMBLOCK_OK;
}

memblock_err_t memblock_reserved_next(memblock_iter_t *iter, range_t *out)
{
  if (!iter || !out || !rb_tree_reserved.root)
    return MEMBLOCK_ERR_INVALID;

  if (!iter->node)
    return MEMBLOCK_ERR_INVALID;

  iter->node = rb_tree_successor(&rb_tree_reserved, iter->node);

  if (!iter->node || iter->node == rb_tree_reserved.NIL)
    return MEMBLOCK_ERR_INVALID;

  *out = *(range_t*)((rb_tree_node_t*)iter->node)->data;
  return MEMBLOCK_OK;
}

static memblock_err_t memblock_add_range(rb_tree_t* tree, range_t physical_range)
{
  range_t merged_range = physical_range;

  range_t search_range = {
    .start = (merged_range.start == 0) ? 0 : merged_range.start - 1,
    .end = (merged_range.end == UINTPTR_MAX) ? UINTPTR_MAX : merged_range.end + 1
  };

  rb_tree_node_t* fnode = memblock_find_overlap(tree, &search_range);

  if (!fnode)
  {
    rb_tree_node_t* node = earlybump_alloc(sizeof(*node), _Alignof(rb_tree_node_t));
    if (!node)
      return MEMBLOCK_ERR_EARLYBUMP;

    range_t* range = earlybump_alloc(sizeof(*range), _Alignof(range_t));
    if (!range)
      return MEMBLOCK_ERR_EARLYBUMP;

    *range = physical_range;
    node->data = range;
    return rb_tree_insert(tree, node) ? MEMBLOCK_ERR_INVALID : MEMBLOCK_OK;
  }

  rb_tree_node_t* reused_node = NULL;
  range_t* reused_range = NULL;

  while ((fnode = memblock_find_overlap(tree, &search_range)))
  {
    range_t* frange = fnode->data;
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

    search_range.start = (merged_range.start == 0) ? 0 : merged_range.start - 1;
    search_range.end = (merged_range.end == UINTPTR_MAX) ? UINTPTR_MAX : merged_range.end + 1;
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

static uint8_t memblock_find_free_in_range(const range_t* memory_range, size_t size, size_t aligned, phys_addr_t* result)
{
  if (!memory_range || !result || size == 0)
    return 0;

  if (memory_range->start > memory_range->end)
    return 0;

  if (aligned == 0)
    aligned = MEMBLOCK_SIZE;

  if ((aligned & (aligned - 1)) != 0)
    return 0;

  phys_addr_t limit = memory_range->end;

  phys_addr_t candidate = ALIGN_UP(memory_range->start, aligned);

  if (candidate < memory_range->start)
    return 0;

  while (candidate <= limit)
  {
    if ((phys_addr_t)(size - 1) > limit - candidate)
      return 0;

    range_t requested = {
      .start = candidate,
      .end = candidate + (phys_addr_t)(size - 1)
    };

    rb_tree_node_t* overlap = memblock_find_overlap(&rb_tree_reserved, &requested);

    if (!overlap)
    {
      *result = candidate;
      return 1;
    }

    range_t* reserved = overlap->data;

    if (!reserved)
      return 0;

    if (reserved->end >= limit)
      return 0;

    if (reserved->end == UINTPTR_MAX)
      return 0;

    phys_addr_t next = reserved->end + 1;

    candidate = ALIGN_UP(next, aligned);

    if (candidate < next)
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
