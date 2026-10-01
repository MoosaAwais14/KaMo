#ifndef KERNEL_RANGE_H
#define KERNEL_RANGE_H

#include <kernel/address.h>

typedef struct range_s {
  addr_t start;
  addr_t end;
} range_t;

static inline uint64_t range_len(const range_t* r1)
{
  if(r1->end < r1->start)
    return 0;
  return (r1->end - r1->start) + 1;
}

static inline uint8_t range_contains(const range_t* r1, const range_t* r2)
{
	return (r1->start <= r2->start) && (r1->end >= r2->end);
}

static inline uint8_t range_overlaps(const range_t* r1, const range_t* r2)
{
	return (r1->start <= r2->end) && (r1->end >= r2->start);
}

#endif
