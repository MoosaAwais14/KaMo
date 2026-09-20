#include <asm/setup.h>

#include <kernel/cpu.h>
#include <kernel/irq.h>
#include <kernel/interrupt.h>

void setup_arch(void)
{
  cpu_early_init(0);
  
  interrupt_init();
  
  // Do some stuff here..

  cpu_init(0);

  irq_init();
}
