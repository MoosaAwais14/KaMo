#ifndef ASM_VMA_H
#define ASM_VMA_H

#if !defined(__ASSEMBLER__) && !defined(LINKER_SCRIPT)

#include <asm/paging.h>

#define VMA_SIZE_SHIFT   PAGE_SHIFT
#define VMA_SIZE         (1UL << VMA_SIZE_SHIFT)

#endif

#endif
