#include <kernel/start_kernel.h>

#include <asm/cpu.h>
#include <asm/irqflags.h>
#include <asm/setup.h>

#include <mm/memblock.h>

boot_info_t kernel_boot_info;

void __noreturn start_kernel(void)
{
  local_irq_disable();

  setup_arch();

  memblock_init();

  char* test = memblock_alloc(100, 0);
  if(test)
    for (char c = 'a'; c <= 'z'; c++)
      test[(int)c - (int)'a'] = c;

  while(1)
  {
    local_irq_disable();
    local_halt();
  }
}
