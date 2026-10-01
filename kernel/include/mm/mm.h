#ifndef MM_MM_H
#define MM_MM_H

#include <kernel/range.h>

#include <mm/vma.h>
#include <mm/mmu.h>

typedef struct mm_s {
    struct mmu_space_s* active_mmu;
    struct vma_space_s* active_vma;
    range_t fixmap_range;
} mm_t;

#endif