#include <asm/mmu.h>

#include <asm/cpu.h>
#include <asm/cpu_arch.h>
#include <asm/page.h>
#include <asm/fixmap.h>

#include <lib/memory.h>

static inline void arch_mmu_decode_flags(mmu_flags_t flags, uint8_t* present, uint8_t* rw, uint8_t* user, uint8_t* pwt, uint8_t* pcd, uint8_t* pat, uint8_t* global);

static inline void arch_mmu_build_huge_pde(page_directory_entry_t* pde, phys_addr_t paddr, uint8_t present, uint8_t rw, uint8_t user, uint8_t pwt, uint8_t pcd, uint8_t pat, uint8_t page_size, uint8_t global);
static inline void arch_mmu_build_pt_pde(page_directory_entry_t *pde, phys_addr_t pt_phys, uint8_t present, uint8_t rw, uint8_t user, uint8_t pwt, uint8_t pcd);
static inline void arch_mmu_build_small_pte(page_table_entry_t* pte, phys_addr_t paddr, uint8_t present, uint8_t rw, uint8_t user, uint8_t pwt, uint8_t pcd, uint8_t pat, uint8_t global);

static inline virt_addr_t arch_mmu_phys_map(struct mmu_space_s* space, phys_addr_t paddr);
static inline void arch_mmu_phys_unmap(struct mmu_space_s* space, virt_addr_t vaddr);

static inline mmu_err_t arch_mmu_map_small(struct mmu_space_s* space, virt_addr_t vaddr, phys_addr_t paddr, mmu_flags_t flags, const mmu_alloc_ops_t* mmu_ops);
static inline mmu_err_t arch_mmu_map_huge(struct mmu_space_s* space, virt_addr_t vaddr, phys_addr_t paddr, mmu_flags_t flags);

mmu_err_t arch_mmu_init(struct mmu_space_s* space)
{
  if(!space || !space->cr3 || !space->is_kernel || !IS_ALIGNED(space->cr3, PAGE_ALIGN))
    return MMU_ERR_BAD_ARG;

  return MMU_OK;
}

mmu_err_t arch_mmu_preallocate_page_table(struct mmu_space_s* space, virt_addr_t vaddr)
{
  if (!space || !IS_ALIGNED(vaddr, PAGE_ALIGN) || !mmu_alloc_ops || !mmu_alloc_ops->alloc_page)
    return MMU_ERR_BAD_ARG;

  page_directory_entry_t* pde = &space->page_directory.entries[PD_INDEX(vaddr)];
  if (pde->ps0.present)
    return pde->ps0.page_size ? MMU_ERR_FIELD_MISMATCH : MMU_OK;

  phys_addr_t pt_phys;
  mmu_err_t alloc_err = mmu_alloc_ops->alloc_page(mmu_alloc_ops->ctx, &pt_phys);
  if (alloc_err != MMU_OK || !IS_ALIGNED(pt_phys, PAGE_ALIGN) || pt_phys > UINT32_MAX) 
  {
    if (alloc_err == MMU_OK && mmu_alloc_ops->release_page)
      mmu_alloc_ops->release_page(mmu_alloc_ops->ctx, pt_phys);

    return MMU_ERR_ALLOCATOR;
  }

  page_table_t* pt = (page_table_t*)arch_mmu_phys_map(space, pt_phys);
  memset(pt, 0, sizeof(*pt));
  arch_mmu_build_pt_pde(pde, pt_phys, 1, 1, 1, 0, 0);
  arch_mmu_phys_unmap(space, (virt_addr_t)pt);

  return MMU_OK;
}

mmu_err_t arch_mmu_tlb_invalidate_page(struct mmu_space_s* space, virt_addr_t vaddr)
{
  if (!space || !IS_ALIGNED(vaddr, PAGE_ALIGN))
    return MMU_ERR_BAD_ARG;

  if (read_cr3() == space->cr3)
    invlpg(vaddr);

  return MMU_OK;
}

mmu_err_t arch_mmu_tlb_flush(struct mmu_space_s* space)
{
  if (!space)
    return MMU_ERR_BAD_ARG;

  if (read_cr3() != space->cr3)
    return MMU_OK;

  const uint32_t cr4 = read_cr4();
  const uint32_t cr4_pge = 1u << 7;
  if (cr4 & cr4_pge) 
  {
    write_cr4(cr4 & ~cr4_pge);
    write_cr4(cr4);
  } 
  else 
  {
    load_cr3(space->cr3);
  }

  return MMU_OK;
}

mmu_err_t arch_mmu_map(struct mmu_space_s* space, virt_addr_t vaddr, phys_addr_t paddr, size_t size, mmu_flags_t flags)
{
  if(!space || !size || !IS_ALIGNED(vaddr, PAGE_ALIGN) ||
     !IS_ALIGNED(paddr, PAGE_ALIGN) || !IS_ALIGNED(size, PAGE_ALIGN) ||
     size > UINTPTR_MAX - vaddr || paddr > UINT32_MAX ||
     (uint64_t)size > (0x100000000ull - paddr))
    return MMU_ERR_BAD_ARG;

  virt_addr_t cur_vaddr = vaddr;
  phys_addr_t cur_paddr = paddr;
  size_t mapped = 0;

  while(mapped < size) 
  {
    const size_t remaining = size - mapped;
    const size_t chunk = (IS_ALIGNED(cur_vaddr, PAGE_ALIGN_HUGE) &&
                          IS_ALIGNED(cur_paddr, PAGE_ALIGN_HUGE) &&
                          remaining >= PAGE_SIZE_HUGE)
                         ? PAGE_SIZE_HUGE
                         : PAGE_SIZE;

    if(chunk == PAGE_SIZE_HUGE) 
    {
      mmu_err_t err = arch_mmu_map_huge(space, cur_vaddr, cur_paddr, flags);
      if(err != MMU_OK) {
        if (mapped)
          arch_mmu_tlb_flush(space);
        return err;
      }
    } 
    else 
    {
      mmu_err_t err = arch_mmu_map_small(space, cur_vaddr, cur_paddr, flags, mmu_alloc_ops);
      if(err != MMU_OK) {
        if (mapped)
          arch_mmu_tlb_flush(space);
        return err;
      }
    }

    cur_vaddr += chunk;
    cur_paddr += chunk;
    mapped += chunk;
  }

  arch_mmu_tlb_flush(space);
  return MMU_OK;
}

mmu_err_t arch_mmu_unmap(struct mmu_space_s* space, virt_addr_t vaddr, size_t size)
{
  if(!space || !size || !IS_ALIGNED(vaddr, PAGE_ALIGN) || !IS_ALIGNED(size, PAGE_ALIGN) ||
     size > UINTPTR_MAX - vaddr)
    return MMU_ERR_BAD_ARG;

  virt_addr_t cur_vaddr = vaddr;
  size_t unmapped = 0;

  while(unmapped < size) 
  {
    const size_t remaining = size - unmapped;
    const size_t pd_idx = PD_INDEX(cur_vaddr);
    page_directory_entry_t* pde = &space->page_directory.entries[pd_idx];

    if (!pde->ps0.present) {
      if (unmapped)
        arch_mmu_tlb_flush(space);
      return MMU_ERR_NOT_MAPPED;
    }

    size_t chunk = PAGE_SIZE;
    if (pde->ps0.page_size)
    {
      if (!IS_ALIGNED(cur_vaddr, PAGE_ALIGN_HUGE) || remaining < PAGE_SIZE_HUGE) {
        if (unmapped)
          arch_mmu_tlb_flush(space);
        return MMU_ERR_FIELD_MISMATCH;
      }

      chunk = PAGE_SIZE_HUGE;
      pde->value = 0;
    } 
    else 
    {
      const size_t pt_idx = PT_INDEX(cur_vaddr);
      phys_addr_t phys_pt = PHYS_PAGE_TABLE(pde);
      if(!phys_pt)
      {
        return MMU_ERR_NOT_MAPPED;
      }

      page_table_t* pt = (page_table_t*)arch_mmu_phys_map(space, phys_pt);
      if(!pt)
      {
        return MMU_ERR_ALLOCATOR;
      }

      if (!pt->pages[pt_idx].bits.present) {
        if (unmapped)
          arch_mmu_tlb_flush(space);
        arch_mmu_phys_unmap(space, (virt_addr_t)pt);
        return MMU_ERR_NOT_MAPPED;
      }
      
      pt->pages[pt_idx].value = 0;

      uint8_t release = 1;
      for (size_t i = 0; i < PAGE_TABLE_ENTRIES; ++i) {
        if (pt->pages[i].bits.present) {
          release = 0;
          break;
        }
      }

      arch_mmu_phys_unmap(space, (virt_addr_t)pt);

      if(release)
      {
        pde->value = 0;
        mmu_alloc_ops->release_page(mmu_alloc_ops->ctx, phys_pt);
      }
    }

    cur_vaddr += chunk;
    unmapped += chunk;
  }

  arch_mmu_tlb_flush(space);
  return MMU_OK;
}

static inline mmu_err_t arch_mmu_map_small(struct mmu_space_s* space, virt_addr_t vaddr, phys_addr_t paddr, mmu_flags_t flags, const mmu_alloc_ops_t* mmu_ops)
{
  if (!space || !mmu_ops || !mmu_ops->alloc_page)
    return MMU_ERR_BAD_ARG;

  const size_t pd_idx = PD_INDEX(vaddr);
  const size_t pt_idx = PT_INDEX(vaddr);
  
  page_directory_entry_t *pde = &space->page_directory.entries[pd_idx];
  
  if (pde->ps0.present && pde->ps0.page_size)
    return MMU_ERR_FIELD_MISMATCH;

  uint8_t present, rw, user, pwt, pcd, pat, global;
  arch_mmu_decode_flags(flags, &present, &rw, &user, &pwt, &pcd, &pat, &global);

  if (!pde->ps0.present) {
    mmu_err_t err = arch_mmu_preallocate_page_table(space, vaddr);
    if (err != MMU_OK)
      return err;
  }

  page_table_t* pt = (page_table_t*)arch_mmu_phys_map(space, PHYS_PAGE_TABLE(pde));

  if(!pt)
    return MMU_ERR_ALLOCATOR;

  page_table_entry_t *pte = &pt->pages[pt_idx];
  arch_mmu_build_small_pte(pte, paddr, present, rw, user, pwt, pcd, pat, global); 

  arch_mmu_phys_unmap(space, (virt_addr_t)pt);

  return MMU_OK;
}

static inline mmu_err_t arch_mmu_map_huge(struct mmu_space_s* space, virt_addr_t vaddr, phys_addr_t paddr, mmu_flags_t flags)
{
  const size_t pd_idx = PD_INDEX(vaddr);

  page_directory_entry_t *pde = &space->page_directory.entries[pd_idx];

  if (pde->ps1.present && !pde->ps1.page_size)
    return MMU_ERR_FIELD_MISMATCH;

  uint8_t present, rw, user, pwt, pcd, pat, global;
  arch_mmu_decode_flags(flags, &present, &rw, &user, &pwt, &pcd, &pat, &global);

  arch_mmu_build_huge_pde(pde, paddr, present, rw, user, pwt, pcd, pat, 1, global);    

  return MMU_OK;
}

static inline virt_addr_t arch_mmu_phys_map(struct mmu_space_s* space, phys_addr_t paddr)
{
  if(paddr < PHYS_DIRECT_MAP_LIMIT)
    return ___va(paddr);

  virt_addr_t vaddr;
  if(arch_fixmap_map(space, paddr, MMU_FLAG_READ | MMU_FLAG_WRITE, &vaddr) != MMU_OK)
    return 0;

  return vaddr;
}

static inline void arch_mmu_phys_unmap(struct mmu_space_s* space, virt_addr_t vaddr)
{
  if(vaddr < DIRECT_MAP_LIMIT)
    return;

  arch_fixmap_unmap(space, vaddr);
}

static inline void arch_mmu_build_huge_pde(page_directory_entry_t* pde, phys_addr_t paddr, uint8_t present, uint8_t rw, uint8_t user, uint8_t pwt, uint8_t pcd, uint8_t pat, uint8_t page_size, uint8_t global)
{
  uint32_t entry = 0;

  entry |= (uint32_t)(paddr & 0xFFC00000);

  entry |= (present & 1u) << 0;
  entry |= (rw      & 1u) << 1;
  entry |= (user    & 1u) << 2;
  entry |= (pwt     & 1u) << 3;
  entry |= (pcd     & 1u) << 4;
  entry |= (1u      & 1u) << 7;
  entry |= (global  & 1u) << 8;
  entry |= (pat     & 1u) << 12;

  pde->value = entry;
}

static inline void arch_mmu_build_pt_pde(page_directory_entry_t *pde, phys_addr_t pt_phys, uint8_t present, uint8_t rw, uint8_t user, uint8_t pwt, uint8_t pcd)
{
  uint32_t entry = 0;

  entry |= (uint32_t)(pt_phys & 0xFFFFF000);

  entry |= (present & 1u) << 0;
  entry |= (rw      & 1u) << 1;
  entry |= (user    & 1u) << 2;
  entry |= (pwt     & 1u) << 3;
  entry |= (pcd     & 1u) << 4;

  pde->value = entry;
}

static inline void arch_mmu_build_small_pte(page_table_entry_t* pte, phys_addr_t paddr, uint8_t present, uint8_t rw, uint8_t user, uint8_t pwt, uint8_t pcd, uint8_t pat, uint8_t global)
{
  struct page_table_entry_s entry = (struct page_table_entry_s){
    .present = present & 1u,
    .rw = rw & 1u,
    .user = user & 1u,
    .pwt = pwt & 1u,
    .pcd = pcd & 1u,
    .accessed = 0,
    .dirty = 0,
    .pat = pat & 1u,
    .global = global & 1u,
    .available = 0,
    .frame = (uint32_t)(paddr >> PAGE_SHIFT)
  };

  pte->bits = entry;
}

static inline void arch_mmu_decode_flags(mmu_flags_t flags, uint8_t* present, uint8_t* rw, uint8_t* user, uint8_t* pwt, uint8_t* pcd, uint8_t* pat, uint8_t* global)
{
  *present = (flags & MMU_FLAG_READ) != 0;
  *rw      = (flags & MMU_FLAG_WRITE) != 0;
  *user    = (flags & MMU_FLAG_USER) != 0;
  *global  = (flags & MMU_FLAG_GLOBAL) != 0;

  *pwt = 0;
  *pcd = 0;
  *pat = 0;

  mmu_flags_t cache = flags & MMU_FLAG_CACHE_MASK;

  if(cache == MMU_FLAG_CACHE_WT)
  {
    *pwt = 1;
  }
  else if(cache == MMU_FLAG_CACHE_UC)
  {
    *pcd = 1;
  }
  else if(cache == MMU_FLAG_CACHE_WC)
  {
    *pwt = 1;
    *pcd = 1;
  }
}
