#include <asm/setup.h>

#include <asm/cpu.h>
#include <asm/cpu_arch.h>
#include <asm/irqflags.h>
#include <asm/exception.h>
#include <asm/sizes.h>
#include <asm/paging.h>
#include <asm/page.h>
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
#include <mm/frame_alloc.h>

static phys_addr_t mmu_bootstrap_alloc_frame(void* __unused__);
static void mmu_bootstrap_free_frame(void* __unused0__, phys_addr_t __unused1__);

static virt_addr_t vma_bootstrap_alloc(void* __unused__, size_t size, size_t aligned);
static void vma_bootstrap_free(void* __unused0__, virt_addr_t __unused1__);

static void setup_bootstrap_paging(void);
static void setup_boot_reserves(void);
static void setup_kernel_reserves(void);
static virt_addr_t setup_bsp_stack(void);

extern void __noreturn arch_switch_stack_to_continue(uint32_t new_stack);

static const mmu_alloc_ops_t mmu_ops = {
  .alloc_frame = mmu_bootstrap_alloc_frame,
  .free_frame = mmu_bootstrap_free_frame,
  .ctx = NULL
};

static const vma_alloc_ops_t vma_ops = {
  .alloc = vma_bootstrap_alloc,
  .free = vma_bootstrap_free,
  .ctx = NULL
};

mmu_space_t kernel_pg_space =  { };
vma_space_t kernel_vma_space = { };
mm_t* kernel_mm = &(mm_t){ .active_mmu = &kernel_pg_space, .active_vma = &kernel_vma_space };

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
  
  setup_boot_reserves();
  setup_kernel_reserves();

  interrupt_init();
  {
    arch_exception_early_init();
  }

  kernel_pg_space.cr3 = ___pa(&kernel_pg_space.page_directory);
  kernel_pg_space.is_kernel = 1;
  if (mmu_early_init(&kernel_pg_space, &mmu_ops) != MMU_OK)
    local_safe_halt();

  cpu_init(0, kernel_mm);

  // setup_bootstrap_paging();

  // patch allocators for mm

  irq_init();

  // arch_exception_init(); // Update "early" exception vectors with proper handling

  if (frame_alloc_init() != FRAME_ALLOC_OK)
    local_safe_halt();

  earlybump_disable();
  continue_start_kernel();
}

static void setup_bootstrap_paging(void)
{
  // do kernel mappings
  // do heap/slab mappings
  load_cr3(kernel_pg_space.cr3);
}

static void setup_boot_reserves(void)
{
  for (size_t i = 0; i < kernel_boot_info.memory_map.count; i++) 
  {
    const boot_info_memory_map_entry_t *e = &kernel_boot_info.memory_map.map[i];

    range_t range = {
      .start = e->start_address,
      .end   = e->end_address,
    };

    if (e->ok)
    {
      memblock_add(range);
    }
    else
    {
      memblock_reserve(range);
    }
  }
}

static void setup_kernel_reserves(void)
{
  memblock_reserve((range_t){ .start = ___pa(_stext),   .end = ___pa(_etext) - 1    });
  memblock_reserve((range_t){ .start = ___pa(_srodata), .end = ___pa(_erodata) - 1  });
  memblock_reserve((range_t){ .start = ___pa(_sdata),   .end = ___pa(_edata) - 1    });
  memblock_reserve((range_t){ .start = ___pa(_sbss),    .end = ___pa(_ebss) - 1     });

  vma_reserve(&kernel_vma_space, (range_t){ .start = (addr_t)_stext,   .end = (addr_t)_etext -1    }, 0);
  vma_reserve(&kernel_vma_space, (range_t){ .start = (addr_t)_srodata, .end = (addr_t)_erodata - 1 }, 0);
  vma_reserve(&kernel_vma_space, (range_t){ .start = (addr_t)_sdata,   .end = (addr_t)_edata - 1   }, 0);
  vma_reserve(&kernel_vma_space, (range_t){ .start = (addr_t)_sbss,    .end = (addr_t)_ebss - 1    }, 0);
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

static phys_addr_t mmu_bootstrap_alloc_frame(void* __unused__)
{
  void* frame = earlybump_alloc(PAGE_SIZE, PAGE_ALIGN);
  if (!frame)
    return 0;

  return ___pa(frame);
}

static void mmu_bootstrap_free_frame(void* __unused0__, phys_addr_t __unused1__) { }

static virt_addr_t vma_bootstrap_alloc(void* __unused__, size_t size, size_t aligned)
{
  void* frame = earlybump_alloc(size, aligned);
  return (virt_addr_t)frame;
}

static void vma_bootstrap_free(void* __unused0__, virt_addr_t __unused1__) { }
