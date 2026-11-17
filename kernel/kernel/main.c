#include <kernel/start_kernel.h>
#include <kernel/interrupt.h>
#include <kernel/cpu.h>
#include <kernel/irq.h>  
#include <kernel/console.h>

#include <mm/mm.h>
#include <mm/slab.h>
#include <mm/mmu.h>
#include <mm/vmalloc.h>
#include <mm/ioremap.h>

#include <asm/cpu.h>
#include <asm/irqflags.h>
#include <asm/setup.h>
#include <asm/exception.h>
#include <lib/string.h>
#include <stdint.h>

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

void vga_print(volatile uint16_t* vga_memory, const char* str)
{
#define VGA_WIDTH  80
#define VGA_HEIGHT 25
#define VGA_COLOR  0x0F

    static int row = 0;
    static int col = 0;

    while (*str)
    {
        if (*str == '\n')
        {
            row++;
            col = 0;
        }
        else
        {
            vga_memory[row * VGA_WIDTH + col] =
                ((uint16_t)VGA_COLOR << 8) | (uint8_t)*str;

            col++;

            if (col >= VGA_WIDTH)
            {
                col = 0;
                row++;
            }
        }

        if (row >= VGA_HEIGHT)
        {
            row = 0;
            col = 0;
        }

        str++;
    }
}

void __noreturn continue_start_kernel(void)
{
  mm_init();
  mmu_set_allocator(&mmu_page_alloc_ops);

  slab_init();
  vmalloc_init();

  if(cpu_init(0, &kernel_mm))
  {
    local_safe_halt();
  }

  // arch_exception_init(); // Update "early" exception vectors with proper handling
  if(irq_init())
  {
    local_safe_halt();
  }

  void* vga = NULL;
  if(!ioremap(0xB8000, 0x1000, MMU_FLAG_READ | MMU_FLAG_WRITE, &vga) && vga)
  {
    vga_print(vga, "KaMo Kernel\n");
    vga_print(vga, "Hello, world!\n");
  }

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
