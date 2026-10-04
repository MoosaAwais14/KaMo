#ifndef ASM_PAGING_H
#define ASM_PAGING_H

#if !defined(__ASSEMBLER__) && !defined(LINKER_SCRIPT)

#include <stdint.h>

#include <asm/page.h>

#include <compiler/attributes.h>

#define PAGE_TABLE_ENTRIES      1024
#define PAGE_DIRECTORY_ENTRIES  1024
#define PAGE_SHIFT              12
#define PAGE_SHIFT_HUGE         22
#define PAGE_SIZE               (1ul << PAGE_SHIFT) 
#define PAGE_ALIGN              (1ul << PAGE_SHIFT)
#define PAGE_SIZE_HUGE          (1ul << PAGE_SHIFT_HUGE)
#define PAGE_ALIGN_HUGE         (1ul << PAGE_SHIFT_HUGE)

struct page_directory_entry_ps0_s {
  uint32_t present      : 1;
  uint32_t rw           : 1;
  uint32_t user         : 1;
  uint32_t pwt          : 1;
  uint32_t pcd          : 1;
  uint32_t accessed     : 1;
  uint32_t padding0     : 1;
  uint32_t page_size    : 1;   
  uint32_t padding1     : 4;
  uint32_t frame        : 20;
};

struct page_directory_entry_ps1_s {
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
};

typedef union {
  uint32_t value;
  struct page_directory_entry_ps0_s ps0;
  struct page_directory_entry_ps1_s ps1;
} page_directory_entry_t;

struct page_table_entry_s {
  uint32_t present     : 1;
  uint32_t rw          : 1;
  uint32_t user        : 1;
  uint32_t pwt         : 1;
  uint32_t pcd         : 1;
  uint32_t accessed    : 1;
  uint32_t dirty       : 1;
  uint32_t pat         : 1;
  uint32_t global      : 1;
  uint32_t available   : 3;
  uint32_t frame       : 20;
};

typedef union {
  uint32_t value;
  struct page_table_entry_s bits;
} page_table_entry_t;

typedef struct page_table_s
{
  page_table_entry_t pages[PAGE_TABLE_ENTRIES];
} page_table_t __aligned(PAGE_ALIGN);

typedef struct page_directory_s
{
  page_directory_entry_t entries[PAGE_DIRECTORY_ENTRIES];
} page_directory_t __aligned(PAGE_ALIGN);

#define PD_INDEX(virt)        ((virt) >> PAGE_SHIFT_HUGE)
#define PT_INDEX(virt)        (((virt) >> PAGE_SHIFT) & (PAGE_TABLE_ENTRIES - 1))
#define PHYS_PAGE_TABLE(pde)  (((page_directory_entry_t*)pde)->ps0.frame << PAGE_SHIFT)

#endif

#endif
