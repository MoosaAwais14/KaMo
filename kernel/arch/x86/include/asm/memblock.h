#ifndef ASM_MEMBLOCK_H
#define ASM_MEMBLOCK_H

#if !defined(__ASSEMBLER__) && !defined(LINKER_SCRIPT)

#define MEMBLOCK_SIZE_SHIFT   12
#define MEMBLOCK_SIZE         (1UL << MEMBLOCK_SIZE_SHIFT)

#endif

#endif
