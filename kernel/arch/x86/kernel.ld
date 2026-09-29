ENTRY(_start)

BASE  = 0x00100000;
KERNEL_VMA = 0xC0000000;

PROVIDE(__PAGE_OFFSET = KERNEL_VMA);

SECTIONS
{
  . = BASE;

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
  . += KERNEL_VMA;

  .text ALIGN(4K) : AT(ADDR(.text) - KERNEL_VMA)
  {
    _stext = .;

    *(.text*)

  }
  _etext = .;

  .rodata ALIGN(4K) : AT(ADDR(.rodata) - KERNEL_VMA)
  {
    _srodata = .;

    *(.rodata*)

  }
  _erodata = .;

  .data ALIGN(4K) : AT(ADDR(.data) - KERNEL_VMA)
  {
    _sdata = .;

    *(.data*)

  }
  _edata = .;

  .bss ALIGN(4K) (NOLOAD) : AT(ADDR(.bss) - KERNEL_VMA)
  {
    _sbss = .;

    *(.bss*)
    *(COMMON)
  }
  _ebss = .;

  . = ALIGN(4K);
}
