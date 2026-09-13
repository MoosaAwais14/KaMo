#include <start_kernel.h>

#include <asm/cpu.h>
#include <asm/irqflags.h>

void __noreturn start_kernel(void)
{
  local_irq_disable();

  

  while(1)
  {
    local_irq_disable();
    local_halt();
  }
}
