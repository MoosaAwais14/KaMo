#ifndef ASM_FRAME_ALLOC_H
#define ASM_FRAME_ALLOC_H

#if !defined(__ASSEMBLER__) && !defined(LINKER_SCRIPT)

#include <asm/paging.h>

#define FRAME_SHIFT       PAGE_SHIFT
#define MAX_PHYSMEM_BITS  32
#define SECTION_SHIFT     29
#define PAGES_PER_SECTION (1ULL << (SECTION_SHIFT - FRAME_SHIFT))

#endif

#endif
