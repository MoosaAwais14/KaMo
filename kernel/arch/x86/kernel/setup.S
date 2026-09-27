  .section .text
  .global arch_switch_stack_to_continue

arch_switch_stack_to_continue:
  movl 4(%esp), %eax
  movl %eax, %esp

  .extern continue_kernel
  call continue_kernel
