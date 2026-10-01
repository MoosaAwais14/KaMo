#ifndef ASM_PAGE_H
#define ASM_PAGE_H

#define __LOAD_PHYSICAL_ADDR  0x00100000
#define __PAGE_OFFSET         0xC0000000

#ifdef LINKER_SCRIPT

PAGE_OFFSET = __PAGE_OFFSET;
LOAD_PHYSICAL_ADDR = __LOAD_PHYSICAL_ADDR;

#else

#define PAGE_OFFSET   (__PAGE_OFFSET)

#ifndef __ASSEMBLER__

#include <kernel/address.h>

#define ___va(x)       ((virt_addr_t)((phys_addr_t)(x) + PAGE_OFFSET))
#define ___pa(x)       ((phys_addr_t)((virt_addr_t)(x) - PAGE_OFFSET))

#define __va(x)        ((void*)___va(x))
#define __pa(x)        (___pa(x))

#endif

#endif

#endif
