#include <mm/mm.h>

#include <mm/earlybump.h>
#include <mm/memblock.h>

#include <asm/cpu.h>
#include <asm/page.h>

void* high_memory = (void*)DIRECT_MAP_LIMIT;

void mm_init_early(void)
{
  earlybump_init();
  memblock_init();
}

void mm_init(void)
{
  if(page_alloc_init() != PAGE_ALLOC_OK)
    local_safe_halt();

  memblock_disable();
  earlybump_disable();
}
