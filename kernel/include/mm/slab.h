#ifndef MM_SLAB_H
#define MM_SLAB_H

#include <stddef.h>

extern void slab_init(void);

extern void* kmalloc(size_t size);
extern void kfree(void* ptr);

#endif
