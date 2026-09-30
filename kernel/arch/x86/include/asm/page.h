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
#define __va(x)		    ((void*)((unsigned long)(x) + PAGE_OFFSET))
#define __pa(x)       ((void*)((unsigned long)(x) - PAGE_OFFSET))
#endif

#endif

#endif
