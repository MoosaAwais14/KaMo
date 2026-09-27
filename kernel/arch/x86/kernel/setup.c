#include <asm/setup.h>

#include <asm/cpu.h>
#include <asm/cpu_arch.h>
#include <asm/irqflags.h>
#include <asm/exception.h>
#include <asm/sizes.h>

#include <kernel/start_kernel.h>
#include <kernel/cpu.h>
#include <kernel/irq.h>
#include <kernel/interrupt.h>

#include <mm/memblock.h>

extern void __noreturn arch_switch_stack_to_continue(uint32_t new_stack);

void __noreturn setup_arch(void)
{
  cpu_early_init(0);

  memblock_init();

  interrupt_init();
  {
    arch_exception_early_init();
  }

  cpu_init(0);

  irq_init();
  // arch_exception_init(); // Update "early" exception vectors with proper handling
  
  cpu_t* cpu = cpu_current();
  arch_cpu_t *arch_cpu = cpu->arch_priv;
  
  arch_cpu->kernel_stack_base = (uintptr_t)memblock_alloc(KERNEL_STACK_SIZE, KERNEL_STACK_ALIGN);
  arch_cpu->kernel_stack = arch_cpu->kernel_stack_base + KERNEL_STACK_SIZE;

  arch_switch_stack_to_continue(arch_cpu->kernel_stack);
}
