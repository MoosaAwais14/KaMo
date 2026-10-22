#ifndef MM_VMALLOC_H
#define MM_VMALLOC_H

#include <stdint.h>
#include <stddef.h>

extern void vmalloc_init(void);

extern void* vmalloc(size_t size);

#endif
