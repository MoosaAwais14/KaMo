#ifndef ASM_SETUP_H
#define ASM_SETUP_H

#if !defined(__ASSEMBLER__) && !defined(LINKER_SCRIPT)

#include <compiler/attributes.h>

extern void __noreturn setup_arch(void);

#endif

#endif
