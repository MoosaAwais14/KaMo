#include <kernel/start_kernel.h>

#include <asm/cpu.h>
#include <asm/irqflags.h>
#include <asm/setup.h>

#include <mm/pmm.h>

boot_info_t kernel_boot_info;

void __noreturn start_kernel(void)
{
  local_irq_disable();

  setup_arch();

  local_safe_halt();
}

void __noreturn continue_kernel(void)
{
  pmm_init();

  local_safe_halt();
}
