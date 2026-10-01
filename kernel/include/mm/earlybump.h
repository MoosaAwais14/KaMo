#ifndef MM_EARLYBUMP_H
#define MM_EARLYBUMP_H

#include <stdint.h>
#include <stddef.h>

#include <lib/sizes.h>

#define EARLYBUMP_RESERVED_SIZE  KiB(256)

extern void earlybump_init(void);
extern void earlybump_disable(void);
extern void* earlybump_alloc(size_t size, size_t aligned);

#endif
