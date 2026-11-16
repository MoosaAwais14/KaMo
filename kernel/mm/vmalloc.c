#include "mm/mm.h"
#include <mm/vmalloc.h>
#include <asm/page.h>
#include <asm/vmalloc.h>
#include <mm/vma.h>
#include <mm/page_alloc.h>
#include <mm/mmu.h>

void vmalloc_init(void) { }

void* vmalloc(size_t size, uint32_t flags)
{
  const range_t vmalloc_range = {
    .start = VMALLOC_START,
    .end = VMALLOC_LIMIT - 1
  };

  vma_node_t* vma = vma_alloc_region(kernel_mm.vma, size, 0, vmalloc_range);
  if(!vma)
    return NULL;

  return (void*)vma->range.start;
}
