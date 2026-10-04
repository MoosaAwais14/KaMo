#ifndef MM_MM_H
#define MM_MM_H

#include <kernel/range.h>

#include <mm/vma.h>
#include <mm/mmu.h>

typedef struct mm_space_s {
  struct mmu_space_s* mmu;
  struct vma_space_s* vma;
} mm_space_t;

#endif
