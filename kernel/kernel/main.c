#include <start_kernel.h>

#include <asm/cpu.h>
#include <asm/irqflags.h>
#include <asm/setup.h>

boot_info_t kernel_boot_info;

void __noreturn start_kernel(void)
{
  local_irq_disable();

  setup_arch(&kernel_boot_info);

  

  while(1)
  {
    local_irq_disable();
    local_halt();
  }
}
