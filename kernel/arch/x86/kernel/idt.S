#include <asm/gdt_arch.h>

  .section .text

.altmacro
.macro GEN_ISR n

isr\n:
  .if (\n == 8)  || \
    (\n == 10) || \
    (\n == 11) || \
    (\n == 12) || \
    (\n == 13) || \
    (\n == 14) || \
    (\n == 17) || \
    (\n == 21) || \
    (\n == 29) || \
    (\n == 30)
    pushl $\n
  .else
    pushl $0
    pushl $\n
  .endif

  jmp ir_common_stub

.endm

.macro GEN_ALL_ISR i, n
  GEN_ISR \i

  .if \i < \n
    GEN_ALL_ISR %(\i + 1), \n
  .endif
.endm

  .extern idt_dispatch
ir_common_stub:
  pusha
  movw %ds, %ax
  pushl %eax
  movw $GDT_KERNEL_DATA32_SELECTOR, %ax
  movw %ax, %ds
  movw %ax, %es
  movw %ax, %fs
  movw %ax, %gs
  pushl %esp
  call idt_dispatch
  addl $4, %esp
  popl %eax
  movw %ax, %ds
  movw %ax, %es
  movw %ax, %fs
  movw %ax, %gs
  popa
  addl $8, %esp
  iret

  .align 4
GEN_ALL_ISR 0, 99
GEN_ALL_ISR 100, 199
GEN_ALL_ISR 200, 255

  .section .data
  .global idt_default_stubs

.macro ADD_ALL_ISR i, n
   .long isr\i

   .if \i < \n
     ADD_ALL_ISR %(\i + 1), \n
   .endif
.endm

  .align 4
idt_default_stubs:
  ADD_ALL_ISR 0, 99
  ADD_ALL_ISR 100, 199
  ADD_ALL_ISR 200, 255

