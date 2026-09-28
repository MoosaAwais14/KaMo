#ifndef MM_MEMBUMP_H
#define MM_MEMBUMP_H

#include <stdint.h>
#include <stddef.h>

#include <sizes.h>

#define MEMBUMP_RESERVED_SIZE  KiB(128)

extern void membump_init(void);
extern void* membump_alloc(size_t size, size_t aligned);

#endif
