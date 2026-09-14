#ifndef KERNEL_INCLUDE_BOOT_INFO_H
#define KERNEL_INCLUDE_BOOT_INFO_H

#include <stdint.h>
#include <stddef.h>

typedef struct boot_info_memory_map_entry_s {
  uint64_t  start_address;
  uint64_t  end_address;
  uint8_t   ok;
} boot_info_memory_map_entry_t;

typedef struct boot_info_memory_map_s {
#define BOOT_INFO_MEMORY_MAP_COUNT_MAX 256
  uint16_t count;
  boot_info_memory_map_entry_t map[BOOT_INFO_MEMORY_MAP_COUNT_MAX];
} boot_info_memory_map_t;

typedef struct boot_info_framebuffer_s {
#define BOOT_INFO_FRAMEBUFFER_TYPE_NONE     0
#define BOOT_INFO_FRAMEBUFFER_TYPE_TEXT     1
#define BOOT_INFO_FRAMEBUFFER_TYPE_GRAPHICS 2
    uint8_t type;

    uint16_t width;
    uint16_t height;
    uint32_t pitch;
    uint8_t bpp;

    uint64_t framebuffer_addr;

    struct {
        uint8_t red_mask_size;
        uint8_t red_field_pos;
        uint8_t green_mask_size;
        uint8_t green_field_pos;
        uint8_t blue_mask_size;
        uint8_t blue_field_pos;
    } rgb;
} boot_info_framebuffer_t;

typedef struct boot_info_firmwares {
    uint64_t acpi_rsdp;
} boot_info_firmware_t;

typedef struct boot_info_s {
#define BOOT_INFO_CMDLINE_MAX 256
  char cmdline[BOOT_INFO_CMDLINE_MAX];

  boot_info_memory_map_t memory_map;

  boot_info_framebuffer_t framebuffer;
  
  boot_info_firmware_t firmware;
} boot_info_t;

#endif
