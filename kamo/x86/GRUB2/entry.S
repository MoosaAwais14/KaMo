  .section .data

  .align 4
stack_bottom:
  .skip 1024
stack_top:

  .section .text
  .global _start

_start:
  cli
  cld

  movl $stack_top, %esp

  pushl %ebx
  pushl %eax

  .extern grub2_start
  call grub2_start
  addl $8, %esp

.hang:
  hlt
  jmp .hang
