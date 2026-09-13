#ifndef KERNEL_INCLUDE_INITCALL_H
#define KERNEL_INCLUDE_INITCALL_H

typedef struct initcall_s {
  int level;
  void* func;
} initcall_t;



#endif
