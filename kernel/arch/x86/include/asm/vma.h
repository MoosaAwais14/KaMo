#ifndef ASM_VMA_H
#define ASM_VMA_H

#if !defined(__ASSEMBLER__) && !defined(LINKER_SCRIPT)

#define VMA_SIZE_SHIFT   12
#define VMA_SIZE         (1UL << VMA_SIZE_SHIFT)

#endif

#endif