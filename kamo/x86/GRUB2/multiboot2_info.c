#include "grub2.h"

#include <lib/string.h>

grub2_err_t multiboot2_to_boot_info(uint32_t magic, uintptr_t ptr, boot_info_t* boot_info)
{
  if(!boot_info || !ptr)
    return GRUB2_ERR_BAD_ARGS;

  memset(boot_info, 0, sizeof(boot_info_t));

  if(MULTIBOOT2_BOOTLOADER_MAGIC != magic)
    return GRUB2_ERR_BAD_MAGIC;

  void* mb2 = (void*)(uintptr_t)ptr;

  struct multiboot_tag* tag;
  for(tag = (void*)((uintptr_t)ptr + 0x08); tag->type != MULTIBOOT_TAG_TYPE_END; tag = (void*)(ALIGN_UP((uintptr_t)tag + tag->size, MULTIBOOT_TAG_ALIGN)))
  {
    if (tag->size == 0)
      break;

    switch(tag->type)
    {
      case MULTIBOOT_TAG_TYPE_CMDLINE:
        {
          struct multiboot_tag_string* cmdline = (void*)tag;
          
          if(strlen(cmdline->string) > BOOT_INFO_CMDLINE_MAX)
            return GRUB2_ERR_CMDLINE_MAXED;

          strcpy(boot_info->cmdline, cmdline->string);

          break;
        }
 
      case MULTIBOOT_TAG_TYPE_MMAP:
        {
          struct multiboot_tag_mmap* mmap = (void*)tag;

          for(multiboot_memory_map_t* entry = mmap->entries; (uintptr_t)entry < (uintptr_t)mmap + mmap->size; entry = (void*)((uintptr_t)entry + mmap->entry_size))
          {
            if(boot_info->memory_map.count >= BOOT_INFO_MEMORY_MAP_COUNT_MAX)
            {
              return GRUB2_ERR_MMAP_MAXED;
            }

            uint64_t address = entry->addr;
            uint64_t length = entry->len;

            if (address > UINTPTR_MAX) {
              continue; 
            }

            if (address + length > ((uint64_t)UINTPTR_MAX + 1)) {
              length = ((uint64_t)UINTPTR_MAX + 1) - address;
            }

            boot_info_memory_map_entry_t* e = &boot_info->memory_map.map[boot_info->memory_map.count++];
            e->start_address = address;
            e->end_address = address + length;
            e->ok = (entry->type == MULTIBOOT_MEMORY_AVAILABLE) ? 1 : 0;
          }

          break;
        }

      case MULTIBOOT_TAG_TYPE_FRAMEBUFFER:
        {
          struct multiboot_tag_framebuffer* fb = (void*)tag;

          boot_info->framebuffer.type = BOOT_INFO_FRAMEBUFFER_TYPE_NONE;

          if(fb->common.framebuffer_addr >= UINTPTR_MAX)
            continue;

          if(fb->common.framebuffer_type == MULTIBOOT_FRAMEBUFFER_TYPE_RGB)
          {
            boot_info->framebuffer.type = BOOT_INFO_FRAMEBUFFER_TYPE_TEXT;

            boot_info->framebuffer.gfx.framebuffer_addr = fb->common.framebuffer_addr;
            boot_info->framebuffer.gfx.width            = fb->common.framebuffer_width;
            boot_info->framebuffer.gfx.height           = fb->common.framebuffer_height;
            boot_info->framebuffer.gfx.pitch            = fb->common.framebuffer_pitch;
            boot_info->framebuffer.gfx.bpp              = fb->common.framebuffer_bpp;

            boot_info->framebuffer.gfx.red_field_pos    = fb->framebuffer_red_field_position;
            boot_info->framebuffer.gfx.red_mask_size    = fb->framebuffer_red_mask_size;
            boot_info->framebuffer.gfx.green_field_pos  = fb->framebuffer_green_field_position;
            boot_info->framebuffer.gfx.green_mask_size  = fb->framebuffer_green_mask_size;
            boot_info->framebuffer.gfx.blue_field_pos   = fb->framebuffer_blue_field_position;
            boot_info->framebuffer.gfx.blue_mask_size   = fb->framebuffer_blue_mask_size;
          }
          else if(fb->common.framebuffer_type == MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT)
          {
            boot_info->framebuffer.type = BOOT_INFO_FRAMEBUFFER_TYPE_GRAPHICS;

            boot_info->framebuffer.text.framebuffer_addr = fb->common.framebuffer_addr;
            boot_info->framebuffer.text.width  = fb->common.framebuffer_width;
            boot_info->framebuffer.text.height = fb->common.framebuffer_height;
            boot_info->framebuffer.text.pitch  = fb->common.framebuffer_pitch;
            boot_info->framebuffer.text.bpp = sizeof(uint16_t) * 8; 
          }

          break;
        }
    }
  }

  return GRUB2_OK;
}
