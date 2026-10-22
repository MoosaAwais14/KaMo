#include <kernel/start_kernel.h>
#include <kernel/interrupt.h>
#include <kernel/irq.h>  

#include <mm/mm.h>
#include <mm/slab.h>
#include <mm/mmu.h>

#include <asm/cpu.h>
#include <asm/irqflags.h>
#include <asm/setup.h>
#include <asm/exception.h>

static mmu_err_t mmu_alloc_page(phys_addr_t* out);
static void mmu_release_page(phys_addr_t paddr);

static mmu_alloc_ops_t mmu_page_alloc_ops = {
  .alloc_page = mmu_alloc_page,
  .release_page = mmu_release_page,
};

boot_info_t kernel_boot_info;

void __noreturn start_kernel(void)
{
  local_irq_disable();

  mm_init_early();
  setup_arch();

  local_safe_halt();
}

void __noreturn continue_start_kernel(void)
{
  mm_init();
  mmu_set_allocator(&mmu_page_alloc_ops);
  slab_init();

  // arch_exception_init(); // Update "early" exception vectors with proper handling
  irq_init();

  local_safe_halt();
}

static mmu_err_t mmu_alloc_page(phys_addr_t *out)
{
  if (!out)
    return MMU_ERR_ALLOCATOR;

  page_t *page = alloc_pages(ZONE_NORMAL, 0);
  if (!page)
    return MMU_ERR_ALLOCATOR;

  page->flags |= PAGE_FLAG_MMU_ALLOCATION;
  *out = PFN_PHYS(page_to_pfn(page));

  return MMU_OK;
}

static void mmu_release_page(phys_addr_t paddr)
{
  if (paddr & (PAGE_SIZE - 1))
    return;

  page_t *page = pfn_to_page(PHYS_PFN(paddr));
  if (!page)
    return;

  if (!(page->flags & PAGE_FLAG_MMU_ALLOCATION))
    return;

  page->flags &= ~PAGE_FLAG_MMU_ALLOCATION;
  free_pages(page, 0);
}
