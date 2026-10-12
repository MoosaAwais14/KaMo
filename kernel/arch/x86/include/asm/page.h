#ifndef ASM_PAGE_H
#define ASM_PAGE_H

#define __LOAD_PHYSICAL_ADDR  0x00100000
#define __PAGE_OFFSET         0xC0000000
#define __DIRECTMAP_LIMIT     0xF0000000
#define __VMALLOC_OFFSET      0x00010000
#define __FIXMAP_LIMIT_END    0xFFFFFFFF

#ifdef LINKER_SCRIPT

PAGE_OFFSET = __PAGE_OFFSET;
LOAD_PHYSICAL_ADDR = __LOAD_PHYSICAL_ADDR;

#else

#define PAGE_SHIFT            12
#define PAGE_SHIFT_HUGE       22
#define PAGE_SIZE             (1ul << PAGE_SHIFT) 
#define PAGE_ALIGN            (1ul << PAGE_SHIFT)
#define PAGE_SIZE_HUGE        (1ul << PAGE_SHIFT_HUGE)
#define PAGE_ALIGN_HUGE       (1ul << PAGE_SHIFT_HUGE)
#define PAGE_MASK             (~(PAGE_SIZE - 1))
#define PAGE_ALIGN_UP(x)      (((unsigned long)(x) + PAGE_SIZE - 1) & PAGE_MASK)
#define PAGE_OFFSET           (__PAGE_OFFSET)

#define DIRECT_MAP_LIMIT      (__DIRECTMAP_LIMIT)
#define PHYS_DIRECT_MAP_LIMIT (PAGE_OFFSET - DIRECT_MAP_LIMIT)

#ifndef __ASSEMBLER__
extern void *high_memory;  
#define VMALLOC_START         (PAGE_ALIGN_UP(high_memory) + __VMALLOC_OFFSET)
#else
#define VMALLOC_START         (DIRECT_MAP_LIMIT + __VMALLOC_OFFSET)
#endif

#define FIXMAP_LIMIT_END      (__FIXMAP_LIMIT_END)

#ifndef __ASSEMBLER__

#include <kernel/address.h>

#define ___va(x)              ((virt_addr_t)((phys_addr_t)(x) + PAGE_OFFSET))
#define ___pa(x)              ((phys_addr_t)((virt_addr_t)(x) - PAGE_OFFSET))

#define __va(x)               ((void*)___va(x))
#define __pa(x)               (___pa(x))

#include <kernel/range.h>

static const range_t phys_direct_map_range = { .start = 0x0, .end = PHYS_DIRECT_MAP_LIMIT - 1 };

#endif

#endif

#endif
