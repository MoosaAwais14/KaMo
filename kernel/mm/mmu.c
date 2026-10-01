#include <mm/mmu.h>

#include <stddef.h>

#include <asm/mmu.h>
#include <asm/page.h>

const mmu_alloc_ops_t* mmu_alloc_ops = NULL;

mmu_err_t mmu_early_init(struct mmu_space_s* space, const mmu_alloc_ops_t* alloc_ops)
{
  if(!space || !alloc_ops)
    return MMU_ERR_BAD_ARG;
    
  mmu_alloc_ops = alloc_ops;

  return arch_mmu_early_init(space);
}

mmu_err_t mmu_patch_early_init(struct mmu_space_s* space, const mmu_alloc_ops_t* new_alloc_ops)
{
  if(!space || !new_alloc_ops)
    return MMU_ERR_BAD_ARG;
  
  mmu_err_t err = arch_mmu_patch_early_init(space, new_alloc_ops);

  if(err)
    return err;

  mmu_alloc_ops = new_alloc_ops;

  return MMU_OK;
}
