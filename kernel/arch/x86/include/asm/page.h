#ifndef ASM_PAGE_H
#define ASM_PAGE_H

extern char __PAGE_OFFSET[];

#define PAGE_OFFSET   ((unsigned long)__PAGE_OFFSET)

#define __va(x)		    ((void*)((unsigned long)(x) + PAGE_OFFSET))
#define __pa(x)       ((void*)((unsigned long)(x) - PAGE_OFFSET))

#endif
