#include <asm/setup.h>

#include <asm/cpu.h>
#include <asm/cpu_arch.h>
#include <asm/irqflags.h>
#include <asm/exception.h>
#include <asm/sizes.h>
#include <asm/paging.h>
#include <asm/page.h>
#include <asm/fixmap.h>
#include <asm/mmu.h>
#include <asm/vma.h>
#include <asm-generic/sections.h>

#include <kernel/start_kernel.h>
#include <kernel/cpu.h>
#include <kernel/irq.h>
#include <kernel/interrupt.h>

#include <mm/earlybump.h>
#include <mm/mmu.h>
#include <mm/vma.h>
#include <mm/memblock.h>
#include <mm/page_alloc.h>

#include <lib/string.h>

static mmu_err_t mmu_bootstrap_alloc_page(void* __unused__, phys_addr_t* out);
static void mmu_bootstrap_release_page(void* __unused__, phys_addr_t paddr);
static mmu_err_t mmu_late_alloc_page(void* __unused__, phys_addr_t* out);
static void mmu_late_release_page(void* __unused__, phys_addr_t paddr);
static virt_addr_t vma_bootstrap_alloc(void* __unused__, size_t size, size_t aligned);
static void vma_bootstrap_free(void* __unused0__, virt_addr_t __unused1__);

static void setup_bootstrap_paging(void);
static void setup_boot_reserves(void);
static int setup_kernel_reserves(void);
static virt_addr_t setup_bsp_stack(void);

extern void __noreturn arch_switch_stack_to_continue(uint32_t new_stack);

static const mmu_alloc_ops_t mmu_ops = {
  .alloc_page = mmu_bootstrap_alloc_page,
  .release_page = mmu_bootstrap_release_page,
  .ctx = NULL
};

static const mmu_alloc_ops_t mmu_late_ops = {
  .alloc_page = mmu_late_alloc_page,
  .release_page = mmu_late_release_page,
  .ctx = NULL
};

static const vma_alloc_ops_t vma_ops = {
  .alloc = vma_bootstrap_alloc,
  .free = vma_bootstrap_free,
  .ctx = NULL
};

mmu_space_t kernel_pg_space =  { };
vma_space_t kernel_vma_space = { };
mm_space_t kernel_mm = {
  .mmu = &kernel_pg_space,
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

  arch_switch_stack_to_continue(bsp_stack);
}

void vga_print(volatile unsigned short *base, const char *str)
{
    static unsigned int pos = 0;

    while (*str)
        base[pos++] = (unsigned char)*str++ | (0x07 << 8);
}

void __noreturn continue_setup_arch(void)
{
  earlybump_init();

  memblock_init();
  vma_set_vma_allocator(&vma_ops);
  if (vma_init(&kernel_vma_space, (range_t) {
    .start = 0x0000000,
    .end = UINTPTR_MAX
  }) != VMA_OK)
    local_safe_halt();

  if (vma_reserve(&kernel_vma_space, (range_t) {
    .start = 0,
    .end = VMA_SIZE - 1
  }, 0) != VMA_OK)
    local_safe_halt();

  if (vma_reserve(&kernel_vma_space, (range_t) {
    .start = FIXMAP_BASE,
    .end = FIXMAP_BASE + FIXMAP_WINDOW_SIZE - 1
  }, 0) != VMA_OK)
    local_safe_halt();

  setup_boot_reserves();
  if (setup_kernel_reserves() != 0)
    local_safe_halt();

  interrupt_init();
  {
    arch_exception_early_init();
  }

  kernel_pg_space.cr3 = ___pa(&kernel_pg_space.page_directory);
  kernel_pg_space.is_kernel = 1;
  if (mmu_init(&kernel_pg_space, &mmu_ops) != MMU_OK)
    local_safe_halt();
  if (arch_fixmap_early_init(&kernel_pg_space) != MMU_OK)
    local_safe_halt();

  cpu_init(0, &kernel_mm);

  setup_bootstrap_paging();

  if (mmu_map(&kernel_pg_space, 0xB8000, 0xB8000, PAGE_SIZE, MMU_FLAG_READ | MMU_FLAG_WRITE) != MMU_OK)
    local_safe_halt();

  if(page_alloc_init() != PAGE_ALLOC_OK)
    local_safe_halt();

  // ISSUE WITH PAGE ALLOC
  mmu_set_alloc_ops(&mmu_late_ops);

  vga_print((void*)0xB8000, "What is good.");
  mmu_unmap(&kernel_pg_space, 0xB8000, PAGE_SIZE);

  // patch allocators for mm

  irq_init();

  // arch_exception_init(); // Update "early" exception vectors with proper handling

  earlybump_disable();
  continue_start_kernel();
}

static void setup_bootstrap_paging(void)
{
  memblock_iter_t iter = { NULL };
  range_t memory;

  if (memblock_memory_first(&iter, &memory) != MEMBLOCK_OK)
    local_safe_halt();

  do 
  {
    if (memory.start >= PHYS_DIRECT_MAP_LIMIT)
      continue;

    const phys_addr_t start = ALIGN_UP(memory.start, PAGE_SIZE);
    const phys_addr_t end = memory.end >= PHYS_DIRECT_MAP_LIMIT ? PHYS_DIRECT_MAP_LIMIT : ALIGN_DOWN(memory.end + 1, PAGE_SIZE);

    if (start >= end)
      continue;

    if (mmu_map(&kernel_pg_space, PAGE_OFFSET + start, start, end - start, MMU_FLAG_READ | MMU_FLAG_WRITE) != MMU_OK)
      local_safe_halt();

  } while (memblock_memory_next(&iter, &memory) == MEMBLOCK_OK);

  range_t kernel_vma_ranges[] = {
    { .start = ALIGN_DOWN((addr_t)_stext, PAGE_SIZE), .end = ALIGN_UP((addr_t)_etext, PAGE_SIZE) - 1 },
    { .start = ALIGN_DOWN((addr_t)_sdata, PAGE_SIZE), .end = ALIGN_UP((addr_t)_edata, PAGE_SIZE) - 1 },
    { .start = ALIGN_DOWN((addr_t)_sbss, PAGE_SIZE), .end = ALIGN_UP((addr_t)_ebss, PAGE_SIZE) - 1 },
  };

  for (size_t i = 0; i < sizeof(kernel_vma_ranges) / sizeof(kernel_vma_ranges[0]); i++)
  {
    if (mmu_map(&kernel_pg_space, kernel_vma_ranges[i].start, ___pa(kernel_vma_ranges[i].start), (kernel_vma_ranges[i].end - kernel_vma_ranges[i].start) + 1, MMU_FLAG_READ | MMU_FLAG_WRITE) != MMU_OK)
      local_safe_halt();
  }

  {
    virt_addr_t ro_start = ALIGN_DOWN((addr_t)_srodata, PAGE_SIZE);
    virt_addr_t ro_end = ALIGN_UP((addr_t)_erodata, PAGE_SIZE) - 1;

    if (mmu_map(&kernel_pg_space, ro_start, ___pa(ro_start), (ro_end - ro_start) + 1, MMU_FLAG_READ) != MMU_OK)
      local_safe_halt();
  }

  load_cr3(kernel_pg_space.cr3);
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
  if (memblock_reserve((range_t){ .start = 0x0, .end = PAGE_SIZE - 1 }) != MEMBLOCK_OK)
    return -1;

  range_t kernel_vma_ranges[] = {
    { .start = ALIGN_DOWN((addr_t)_stext, PAGE_SIZE), .end = ALIGN_UP((addr_t)_etext, PAGE_SIZE) - 1 },
    { .start = ALIGN_DOWN((addr_t)_srodata, PAGE_SIZE), .end = ALIGN_UP((addr_t)_erodata, PAGE_SIZE) - 1 },
    { .start = ALIGN_DOWN((addr_t)_sdata, PAGE_SIZE), .end = ALIGN_UP((addr_t)_edata, PAGE_SIZE) - 1 },
    { .start = ALIGN_DOWN((addr_t)_sbss, PAGE_SIZE), .end = ALIGN_UP((addr_t)_ebss, PAGE_SIZE) - 1 }
  };

  for (size_t i = 0; i < sizeof(kernel_vma_ranges) / sizeof(kernel_vma_ranges[0]); i++)
  {
    if(memblock_reserve((range_t){ .start = ___pa(kernel_vma_ranges[i].end), .end = ___pa(kernel_vma_ranges[i].end) }) != MEMBLOCK_OK)
      return -1;

    if (vma_reserve(&kernel_vma_space, kernel_vma_ranges[i], 0) != VMA_OK)
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

static mmu_err_t mmu_bootstrap_alloc_page(void* __unused__, phys_addr_t* out)
{
  range_t range;
  if(memblock_alloc_range(PAGE_SIZE, PAGE_ALIGN, phys_direct_map_range, &range) != MEMBLOCK_OK)
    return MMU_ERR_ALLOCATOR;

  *out = range.start;
  return MMU_OK;
}

static void mmu_bootstrap_release_page(void* __unused__, phys_addr_t paddr) 
{
  range_t range = { .start = paddr, .end = (paddr + PAGE_SIZE) - 1 };
  memblock_alloc_free(range);
}

static mmu_err_t mmu_late_alloc_page(void* __unused__, phys_addr_t* out)
{
  page_t* page = alloc_pages(ZONE_NORMAL, 0);
  if(!page)
    return MMU_ERR_ALLOCATOR;

  pfn_t pfn = page_to_pfn(page);
  *out = PFN_PHYS(pfn);

  return MMU_OK;
}

static void mmu_late_release_page(void* __unused__, phys_addr_t paddr)
{
  page_t* page = pfn_to_page(PHYS_PFN(paddr));
  if(page->refcount > 0)
    page->refcount--;
  if(page->refcount == 0)
    free_pages(page, 0);
}

static virt_addr_t vma_bootstrap_alloc(void* __unused__, size_t size, size_t aligned)
{
  void* frame = earlybump_alloc(size, aligned);
  return (virt_addr_t)frame;
}

static void vma_bootstrap_free(void* __unused0__, virt_addr_t __unused1__) { }
