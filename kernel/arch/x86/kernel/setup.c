#include <asm/setup.h>

#include <asm/cpu.h>
#include <asm/cpu_arch.h>
#include <asm/irqflags.h>
#include <asm/exception.h>
#include <asm/sizes.h>
#include <asm/paging.h>
#include <asm/page.h>
#include <asm/fixmap.h>
#include <asm/vmalloc.h>
#include <asm/mmu.h>
#include <asm/vma.h>
#include <asm-generic/sections.h>

#include <kernel/start_kernel.h>
#include <kernel/cpu.h>

#include <mm/mm.h>
#include <mm/earlybump.h>
#include <mm/mmu.h>
#include <mm/vma.h>
#include <mm/memblock.h>

#include <lib/string.h>

static mmu_err_t mmu_bootstrap_alloc_page(phys_addr_t* out);
static void mmu_bootstrap_release_page(phys_addr_t paddr);

static void setup_bootstrap_paging(void);
static void setup_boot_reserves(void);
static int setup_kernel_reserves(void);
static virt_addr_t setup_bsp_stack(void);

extern void __noreturn arch_switch_stack_to_continue(uint32_t new_stack);

static mmu_alloc_ops_t early_mmu_alloc_ops = {
  .alloc_page = mmu_bootstrap_alloc_page,
  .release_page = mmu_bootstrap_release_page,
};

static mmu_space_t kernel_pgd_space =  { };
static vma_space_t kernel_vma_space = { };

mm_space_t kernel_mm = {
  .mmu = &kernel_pgd_space,
  .vma = &kernel_vma_space
};

void __noreturn setup_arch(void)
{
  cpu_early_init(0);

  virt_addr_t bsp_stack = setup_bsp_stack();
  if(!bsp_stack)
  {
    local_safe_halt();
  }

  interrupt_init();
  {
    arch_exception_early_init();
  }

  arch_switch_stack_to_continue(bsp_stack);
}

void __noreturn continue_setup_arch(void)
{
  setup_boot_reserves();
  if (setup_kernel_reserves() != 0)
    local_safe_halt();

  kernel_pgd_space.arch.cr3 = ___pa(&kernel_pgd_space.arch.page_directory);
  kernel_pgd_space.is_kernel = 1;

  mmu_set_allocator(&early_mmu_alloc_ops);

  if (mmu_init(&kernel_pgd_space) != MMU_OK)
    local_safe_halt();

  if (arch_fixmap_init(&kernel_pgd_space.arch) != MMU_OK)
    local_safe_halt();

  setup_bootstrap_paging();
  continue_start_kernel();
}

static void setup_bootstrap_paging(void)
{
  const range_t kernel_vm_ranges[] = {
    { .start = ALIGN_DOWN((addr_t)_stext, PAGE_SIZE), .end = ALIGN_UP((addr_t)_etext, PAGE_SIZE) - 1 },
    { .start = ALIGN_DOWN((addr_t)_sdata, PAGE_SIZE), .end = ALIGN_UP((addr_t)_edata, PAGE_SIZE) - 1 },
    { .start = ALIGN_DOWN((addr_t)_sbss, PAGE_SIZE), .end = ALIGN_UP((addr_t)_ebss, PAGE_SIZE) - 1 },
    { .start = ALIGN_DOWN((addr_t)_srodata, PAGE_SIZE), .end = ALIGN_UP((addr_t)_erodata, PAGE_SIZE) - 1 }
  };

  memblock_iter_t iter = { NULL };
  range_t memory;

  if (memblock_memory_first(&iter, &memory) != MEMBLOCK_OK)
    local_safe_halt();

  do 
  {
    if (memory.start >= PHYS_DIRECT_MAP_LIMIT)
      continue;

    const phys_addr_t start = ALIGN_UP(memory.start, PAGE_SIZE);
    const phys_addr_t end = (memory.end >= PHYS_DIRECT_MAP_LIMIT ? PHYS_DIRECT_MAP_LIMIT : ALIGN_DOWN(memory.end + 1, PAGE_SIZE)) - 1;

    if (start >= (end + 1))
      continue;

    if (mmu_map(&kernel_pgd_space, ___va(start), start, (end - start) + 1, MMU_FLAG_READ | MMU_FLAG_WRITE) != MMU_OK)
      local_safe_halt();

    high_memory = __va(end) + 1;
  } while (memblock_memory_next(&iter, &memory) == MEMBLOCK_OK);

  for (size_t i = 0; i < 3; i++)
  {
    if (mmu_map(&kernel_pgd_space, kernel_vm_ranges[i].start, ___pa(kernel_vm_ranges[i].start), (kernel_vm_ranges[i].end - kernel_vm_ranges[i].start) + 1, MMU_FLAG_READ | MMU_FLAG_WRITE) != MMU_OK)
      local_safe_halt();
  }

  if (mmu_map(&kernel_pgd_space, kernel_vm_ranges[3].start, ___pa(kernel_vm_ranges[3].start), (kernel_vm_ranges[3].end - kernel_vm_ranges[3].start) + 1, MMU_FLAG_READ) != MMU_OK)
    local_safe_halt();

  load_cr3(kernel_pgd_space.arch.cr3);
}

static void setup_boot_reserves(void)
{
  for (size_t i = 0; i < kernel_boot_info.memory_map.count; i++) 
  {
    const boot_info_memory_map_entry_t *e = &kernel_boot_info.memory_map.map[i]; 

    if (e->type == BOOT_INFO_MEMORY_TYPE_USABLE)
    {
      memblock_add(e->phys_range);
    }
    else
    {
      memblock_reserve(e->phys_range);
    }
  }
}

static int setup_kernel_reserves(void)
{
  const range_t kernel_vm_ranges[] = {
    { .start = ALIGN_DOWN((addr_t)_stext, PAGE_SIZE), .end = ALIGN_UP((addr_t)_etext, PAGE_SIZE) - 1 },
    { .start = ALIGN_DOWN((addr_t)_sdata, PAGE_SIZE), .end = ALIGN_UP((addr_t)_edata, PAGE_SIZE) - 1 },
    { .start = ALIGN_DOWN((addr_t)_sbss, PAGE_SIZE), .end = ALIGN_UP((addr_t)_ebss, PAGE_SIZE) - 1 },
    { .start = ALIGN_DOWN((addr_t)_srodata, PAGE_SIZE), .end = ALIGN_UP((addr_t)_erodata, PAGE_SIZE) - 1 }
  };

  if (memblock_reserve((range_t){ .start = 0x0, .end = PAGE_SIZE - 1 }) != MEMBLOCK_OK)
    return -1;

  for (size_t i = 0; i < sizeof(kernel_vm_ranges) / sizeof(kernel_vm_ranges[0]); i++)
  {
    if(memblock_reserve((range_t){ .start = ___pa(kernel_vm_ranges[i].start), .end = ___pa(kernel_vm_ranges[i].end) }) != MEMBLOCK_OK)
      return -1;
  }

  return 0;
}

static virt_addr_t setup_bsp_stack(void)
{
  extern char __stack_bottom[], __stack_top[];

  cpu_t* cpu = cpu_current();
  arch_cpu_t *arch_cpu = cpu->arch_priv;

  arch_cpu->kernel_stack_base = (virt_addr_t)__stack_bottom;
  arch_cpu->kernel_stack = (virt_addr_t)__stack_top;
  return arch_cpu->kernel_stack;
}

static mmu_err_t mmu_bootstrap_alloc_page(phys_addr_t* out)
{
  range_t range;
  if(memblock_alloc_range(PAGE_SIZE, PAGE_SIZE, phys_direct_map_range, &range) != MEMBLOCK_OK)
    return MMU_ERR_ALLOCATOR;

  *out = range.start;
  return MMU_OK;
}

static void mmu_bootstrap_release_page(phys_addr_t paddr) 
{
  range_t range = { .start = paddr, .end = (paddr + PAGE_SIZE) - 1 };
  memblock_alloc_free(range);
}
