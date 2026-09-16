#include <asm/setup.h>

#include <cpu.h>

void setup_arch(const boot_info_t* boot_info)
{
  cpu_early_init(0);

  // .....
}
