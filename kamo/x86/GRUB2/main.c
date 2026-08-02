#include "grub2.h"

void grub2_start(uint32_t magic, uint32_t ptr)
{
  
}

__boot grub2_multiboot_header_t multiboot2_header = {
  .header = GRUB2_MULTIBOOT_HEADER(0, sizeof(multiboot2_header)),
  .tags_raw = {
    GRUB2_MULTIBOOT_HEADER_TAG(0, 0, 8)
  }
};
