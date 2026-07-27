#include "grub2.h"

void grub2_start(uint32_t magic, uint32_t ptr)
{
  
}

__boot __attribute__((aligned(8))) const uint8_t multiboot_compliant_header[256] = {
  MULTIBOOT2_HEADER(
    MULTIBOOT2_HEADER_MAGIC,
    0x00000000,
    sizeof(multiboot_compliant_header)
  ),

  MULTIBOOT2_HEADER_TAG(0, 0, 8)
};
