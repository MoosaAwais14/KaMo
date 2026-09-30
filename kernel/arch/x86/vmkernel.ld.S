#include <asm/page.h>
#include <asm/sizes.h>

ENTRY(_start)

SECTIONS
{
  . = LOAD_PHYSICAL_ADDR;

  .boot.text ALIGN(4K) :
  {
    *(.boot.text)
  }

  .boot.rodata ALIGN(4K) :
  {
    *(.boot.rodata)
  }

  .boot.data ALIGN(4K) :
  {
    *(.boot.data)
  }

  .boot.bss ALIGN(4K) (NOLOAD) :
  {
    *(.boot.bss)
  }

  . = ALIGN(4K);
  . += PAGE_OFFSET;

  .text ALIGN(4K) : AT(ADDR(.text) - PAGE_OFFSET)
  {
    _stext = .;

    *(.text*)
  }
  _etext = .;

  .rodata ALIGN(4K) : AT(ADDR(.rodata) - PAGE_OFFSET)
  {
    _srodata = .;

    *(.rodata*)
  }
  _erodata = .;

  .data ALIGN(4K) : AT(ADDR(.data) - PAGE_OFFSET)
  {
    _sdata = .;

    *(.data*)

    . = ALIGN(4K);
    __stack_bottom = .;
    . += KERNEL_STACK_SIZE;
    . = ALIGN(KERNEL_STACK_ALIGN);
    __stack_top = .;
  }
  _edata = .;

  .bss ALIGN(4K) (NOLOAD) : AT(ADDR(.bss) - PAGE_OFFSET)
  {
    _sbss = .;

    *(.bss*)
    *(COMMON)
  }
  _ebss = .;

  . = ALIGN(4K);
}