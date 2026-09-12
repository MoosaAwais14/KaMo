#include "grub2.h"

#include <asm/cpu.h>

boot_info_t grub2_boot_info = {

};

static const char *cmdline_get(const char *cmdline, const char *name);

void grub2_start(uint32_t magic, uint32_t ptr)
{
  x86_cpu_cli();

  grub2_err_t err;

  if((err = multiboot2_to_boot_info(magic, ptr, &grub2_boot_info)) != GRUB2_OK)
  {
    return;
  }
  
  err = uart_init(0x3F8, 9600);
  puts("UART Enabled (io:0x3F8 baud:9600)\r\n");
  
  puts("\r\n");

  puts("UART Shutting Down\r\n");
  uart_shutdown();
}

__boot grub2_multiboot_header_t multiboot2_header = {
  .header = GRUB2_MULTIBOOT_HEADER(0, sizeof(multiboot2_header)),
  .tags_raw = {
    GRUB2_MULTIBOOT_HEADER_TAG(0, 0, 8)
  }
};
