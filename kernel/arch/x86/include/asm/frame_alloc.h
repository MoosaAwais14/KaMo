#ifndef ASM_FRAME_ALLOC_H
#define ASM_FRAME_ALLOC_H

#if !defined(__ASSEMBLER__) && !defined(LINKER_SCRIPT)

#define FRAME_SIZE_SHIFT   12
#define FRAME_SIZE         (1UL << FRAME_SIZE_SHIFT)

#endif

#endif

