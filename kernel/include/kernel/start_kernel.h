#ifndef KERNEL_START_KERNEL_H
#define KERNEL_START_KERNEL_H

#include <attributes.h>

#include <kernel/boot_info.h>

extern boot_info_t kernel_boot_info;

extern void __noreturn start_kernel(void);

#endif
