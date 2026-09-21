#include <asm/setup.h>

#include <asm/exception.h>

#include <kernel/cpu.h>
#include <kernel/irq.h>
#include <kernel/interrupt.h>

void setup_arch(void)
{
  cpu_early_init(0);
  
  interrupt_init();
  {
    // Add interrupt vectors
    arch_exception_early_init();
  }

  // Do some stuff here..

  cpu_init(0);

  irq_init();
  // arch_exception_init(); // Update "early" exception vectors with proper handling
}
