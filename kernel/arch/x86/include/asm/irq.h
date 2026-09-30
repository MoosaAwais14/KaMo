#ifndef ASM_IRQ_H
#define ASM_IRQ_H

#if !defined(__ASSEMBLER__) && !defined(LINKER_SCRIPT)

extern int arch_irq_init(void);

#endif

#endif
