#include <asm/setup.h>

#include <asm/cpu.h>
#include <asm/cpu_arch.h>
#include <asm/irqflags.h>
#include <asm/exception.h>
#include <asm/sizes.h>
#include <asm/page.h>
#include <asm-generic/sections.h>

#include <kernel/start_kernel.h>
#include <kernel/cpu.h>
#include <kernel/irq.h>
#include <kernel/interrupt.h>

#include <mm/earlybump.h>
#include <mm/memblock.h>

static void setup_kernel_reserves(void)
{
  memblock_reserve((range_t){ .start = (uint64_t)__pa(_stext), .end =  (uint64_t)__pa(_etext)});
  memblock_reserve((range_t){ .start = (uint64_t)__pa(_srodata), .end =  (uint64_t)__pa(_erodata)});
  memblock_reserve((range_t){ .start = (uint64_t)__pa(_sdata), .end =  (uint64_t)__pa(_edata)});
  memblock_reserve((range_t){ .start = (uint64_t)__pa(_sbss), .end =  (uint64_t)__pa(_ebss)});
}

extern void __noreturn arch_switch_stack_to_continue(uint32_t new_stack);

void __noreturn setup_arch(void)
{
  cpu_early_init(0);

  earlybump_init();

  memblock_init();


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

  setup_kernel_reserves();

  cpu_t* cpu = cpu_current();
  arch_cpu_t *arch_cpu = cpu->arch_priv;

  arch_cpu->kernel_stack_base = (uintptr_t)memblock_alloc(KERNEL_STACK_SIZE);
  arch_cpu->kernel_stack = arch_cpu->kernel_stack_base + KERNEL_STACK_SIZE;

  arch_switch_stack_to_continue(arch_cpu->kernel_stack);
}

void __noreturn continue_setup_arch(void)
{
  interrupt_init();
  {
    arch_exception_early_init();
  }

  cpu_init(0);

  irq_init();

  // arch_exception_init(); // Update "early" exception vectors with proper handling

  earlybump_disable();
  continue_start_kernel();
}
