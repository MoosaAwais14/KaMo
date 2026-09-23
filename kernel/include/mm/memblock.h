#ifndef MM_MEMBLOCK_H
#define MM_MEMBLOCK_H

#include <stdint.h>
#include <stddef.h>

#include <sizes.h>

#define MEMBLOCK_RESERVED_SIZE  MiB(1)
#define MEMBLOCK_REGION_WIDTH   256

extern void memblock_init(void);
extern void* memblock_alloc(size_t size, size_t aligned);

#endif
