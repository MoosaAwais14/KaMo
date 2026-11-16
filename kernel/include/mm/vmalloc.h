#ifndef MM_VMALLOC_H
#define MM_VMALLOC_H

#include <stdint.h>
#include <stddef.h>

#include <mm/vma.h>
#include <mm/mmu_types.h>
#include <mm/page_alloc.h>

extern void vmalloc_init(void);

extern void* vmap(page_t** pages, size_t page_count, mmu_flags_t mmu_flags, vma_flags_t vma_flags);
extern void* vmalloc(size_t size);
extern void vfree(void* addr);

#endif
