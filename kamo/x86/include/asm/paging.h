#ifndef KAMO_X86_INCLUDE_ASM_PAGING_H
#define KAMO_X86_INCLUDE_ASM_PAGING_H

#include <stdint.h>
#include <stddef.h>

#include <attributes.h>

#define PAGE_DIRECTORY_ENTRIES  1024

typedef struct page_directory_entry_ps1_s {
  uint32_t present      : 1;
  uint32_t rw           : 1;
  uint32_t user         : 1;
  uint32_t pwt          : 1;
  uint32_t pcd          : 1;
  uint32_t accessed     : 1;
  uint32_t dirty        : 1;
  uint32_t page_size    : 1; 
  uint32_t global       : 1;
  uint32_t available    : 3;
  uint32_t pat          : 1;
  uint32_t reserved     : 9;
  uint32_t frame        : 10;
} page_directory_entry_ps1_t;

typedef union {
  uint32_t value;
  struct page_directory_entry_ps1_s ps1;
} page_directory_entry_t;

typedef struct page_directory_s
{
  page_directory_entry_t entries[PAGE_DIRECTORY_ENTRIES];
} page_directory_t __aligned(4096);

static __always_inline void x86_cpu_write_cr3(uint32_t cr3)
{
  __asm__ volatile("mov %0, %%cr3" ::"r"(cr3));
}

static __always_inline uint32_t x86_cpu_read_cr3(void)
{
  uint32_t val;
  __asm__ volatile ("mov %%cr3, %0" : "=r"(val));
  return val;
}

#endif
