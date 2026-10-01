#include <asm/mmu.h>

#include <asm/cpu.h>
#include <asm/page.h>

#include <lib/memory.h>

static inline void mmu_decode_flags(mmu_flags_t flags, uint8_t* rw, uint8_t* user, uint8_t* wt, uint8_t* nocache, uint8_t* pat);

static inline mmu_err_t arch_mmu_map_small(struct mmu_space_s* space, virt_addr_t vaddr, phys_addr_t paddr, size_t size, mmu_flags_t flags, const mmu_alloc_ops_t* mmu_ops);

mmu_err_t arch_mmu_early_init(struct mmu_space_s* space)
{
  if (!space || !space->cr3 || !space->is_kernel || !IS_ALIGNED(space->cr3, PAGE_ALIGN))
    return MMU_ERR_BAD_ARG;

  return MMU_OK;
}

mmu_err_t arch_mmu_patch_early_init(struct mmu_space_s* space, const mmu_alloc_ops_t* new_alloc_ops)
{
  if (!space || !space->cr3 || !space->is_kernel || !IS_ALIGNED(space->cr3, PAGE_ALIGN) || !new_alloc_ops)
    return MMU_ERR_BAD_ARG;
  
  for(size_t pd_idx = 0; pd_idx < PAGE_DIRECTORY_ENTRIES; pd_idx++)
  {
    page_directory_entry_t* pde = &space->page_directory.entries[pd_idx];
    if(pde->ps0.page_size)
      continue;
  
    phys_addr_t phys_pt = PAGE_TABLE(pde);  
    if(!phys_pt)
      continue;
  
    // convert phys_pt to virtual address (either through temp
    // mapping or regular DIRECT_MAP arithmetic).
    // allocate a new page_table physical frame and copy data over.
    // finally replace pde frame
  }

  return MMU_OK;
}

mmu_err_t arch_mmu_map(struct mmu_space_s* space, virt_addr_t vaddr, phys_addr_t paddr, size_t size, mmu_flags_t flags)
{
  if(!space)
    return MMU_ERR_BAD_ARG;

  if(!IS_ALIGNED(vaddr, PAGE_ALIGN) || !IS_ALIGNED(paddr, PAGE_ALIGN))
    return MMU_ERR_BAD_ARG;

  return arch_mmu_map_small(space, vaddr, paddr, size, flags, mmu_alloc_ops);
}

static inline mmu_err_t arch_mmu_map_small(struct mmu_space_s* space, virt_addr_t vaddr, phys_addr_t paddr, size_t size, mmu_flags_t flags, const mmu_alloc_ops_t* mmu_ops)
{
  const size_t pd_idx = PD_INDEX(vaddr);
  const size_t pt_idx = PT_INDEX(vaddr);
  
  page_directory_entry_t *pde = &space->page_directory.entries[pd_idx];

  if(!pde->ps0.present)
  {
    phys_addr_t pt_phys = mmu_ops->alloc_frame(mmu_ops->ctx);

    if(!pt_phys)
      return MMU_ERR_ALLOCATOR;
    
  }

  return MMU_OK;
}

static inline void mmu_decode_flags(mmu_flags_t flags, uint8_t* rw, uint8_t* user, uint8_t* wt, uint8_t* nocache, uint8_t* pat)
{
  *rw      = (flags & MMU_FLAG_WRITE) != 0;
  *user    = (flags & MMU_FLAG_USER) != 0;

  *wt = 0;
  *nocache = 0;
  *pat = 0;

  mmu_flags_t cache = flags & MMU_FLAG_CACHE_MASK;

  switch (cache)
  {
    case MMU_FLAG_CACHE_WT:
      *wt = 1;
      break;

    case MMU_FLAG_CACHE_UC:
      *nocache = 1;
      break;

    case MMU_FLAG_CACHE_WC:
      *wt = 1;
      *nocache = 1;
      break;

    case MMU_FLAG_CACHE_WB:
    default:
      break;
  }
}
