#include <mm/vma.h>
#include <asm/vma.h>
#include <kernel/range.h>
#include <lib/memory.h>

static int memblock_rb_vma_cmp(const void* a, const void* b);

void vma_init(vma_space_t* vma_space, range_t virtual_limits)
{
  if(!vma_space)
    return;

  rb_tree_init(&vma_space->busy_tree, memblock_rb_vma_cmp);
  vma_space->rlock = RAW_SPINLOCK_UNLOCKED;
  vma_space->limits = virtual_limits;
}

static int memblock_rb_vma_cmp(const void* a, const void* b)
{
  const vma_node_t* node_a = a;
  const vma_node_t* node_b = b;

  if(node_a->range.start < node_b->range.start)
    return -1;

  if(node_a->range.start > node_b->range.start)
    return 1;

  return 0;
}
