#include "grub.h"

#include <attributes.h>
#include <kernel/start_kernel.h>

#include <asm/cpu.h>
#include <asm/paging.h>
#include <lib/string.h>
#include <lib/stdlib.h>

static void fail(const char* file, int line, const char* msg);
static grub_err_t check_cpu_support(void);
static void create_page_directory(void);
static void enable_paging(void);

static boot_info_t grub2_boot_info = { 0 };
static page_directory_t grub2_page_directory = { 0 };

#define FAIL(msg) \
  fail(__FILE__, __LINE__, msg)

void grub2_start(uint32_t magic, uint32_t ptr)
{
  x86_cpu_cli();

  grub_err_t err;

  if((err = multiboot2_to_boot_info(magic, ptr, &grub2_boot_info)) != GRUB_OK)
    FAIL("multiboot2_to_boot_info");

  err = uart_init(0x3F8, 9600);
  puts("\nBooted to x86/grub\n\n");
  puts("UART Enabled (io:0x3F8 baud:9600)\n");

  if((err = check_cpu_support()) != GRUB_OK)
    FAIL("check_cpu_support");

  create_page_directory();
  puts("Created page_directory\n");

  enable_paging();
  puts("Enabled paging\n");
  
  puts("Copying boot_info to kernel\n");
  memcpy(&kernel_boot_info, &grub2_boot_info, sizeof(boot_info_t));

  puts("\nReady to go to kernel\n\n");

  puts("UART Shut down\n\n");
  uart_shutdown();
  
  start_kernel();
}

static void fail(const char* file, int line, const char* msg)
{
  static char buff[64];
  puts("\nFailed\n\nFile:");
  puts(file);
  puts("\nLine:");
  puts(itoa(line, buff, 10));
  puts("\n\n");
  puts(msg);

  for(;;)
    x86_cpu_halt();
}

static grub_err_t check_cpu_support(void)
{
  if(!x86_cpu_pse_support())
  {
    puts("CPU does not support PSE\n");
    return GRUB_ERR_GENERIC;
  }

  puts("CPU passed checks\n");
  return GRUB_OK;
}

static void create_page_directory(void)
{
  extern char KERNEL_VMA[];
  uintptr_t kernel_vma = (uintptr_t)KERNEL_VMA;
  size_t kernel_vma_idx = kernel_vma >> 22u;

  memset(&grub2_page_directory, 0, sizeof(grub2_page_directory));

  size_t i;

  for(i = 0; i < kernel_vma_idx; i++)
  {
    uint32_t pa = (uint32_t)(i << 22);

    grub2_page_directory.entries[i].ps1.present = 1;
    grub2_page_directory.entries[i].ps1.page_size = 1;
    grub2_page_directory.entries[i].ps1.rw = 1;

    grub2_page_directory.entries[i].value |= pa;
  }

  for(i = kernel_vma_idx; i < PAGE_DIRECTORY_ENTRIES; i++)
  {
    uint32_t pa = (uint32_t)((i - kernel_vma_idx) << 22);

    grub2_page_directory.entries[i].ps1.present = 1;
    grub2_page_directory.entries[i].ps1.page_size = 1;
    grub2_page_directory.entries[i].ps1.rw = 1;

    grub2_page_directory.entries[i].value |= pa;
  }
}

static void enable_paging(void)
{
  uint32_t cr4 = x86_cpu_read_cr4();
  cr4 |= (1 << 4);
  x86_cpu_write_cr4(cr4);

  uint32_t page_directory = (uint32_t)&grub2_page_directory;
  x86_cpu_write_cr3(page_directory);

  uint32_t cr0 = x86_cpu_read_cr0();
  cr0 |= (1 << 31);
  x86_cpu_write_cr0(cr0);
}

__boot grub_multiboot_header_t multiboot2_header = {
  .header = GRUB_MULTIBOOT_HEADER(0, sizeof(multiboot2_header)),
  .tags_raw = {
    GRUB_MULTIBOOT_HEADER_TAG(0, 0, 8)
  }
};
