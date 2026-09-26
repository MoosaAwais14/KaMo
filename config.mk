ifndef ARCH
$(error ARCH is required. Usage: make ARCH=<architecture> KERNEL_BOOTLOADER=<bootloader>)
endif

PROJECT_ROOT := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
ARCH_CONFIG := $(PROJECT_ROOT)config/arch/$(ARCH).mk
ifeq ($(wildcard $(ARCH_CONFIG)),)
$(error Unsupported ARCH '$(ARCH)': expected configuration at $(ARCH_CONFIG))
endif
include $(ARCH_CONFIG)

ifeq ($(origin CROSS_COMPILE),undefined)
$(error $(ARCH_CONFIG) must define CROSS_COMPILE; use an empty value for host tools)
endif

ifndef KERNEL_BOOTLOADER
$(error KERNEL_BOOTLOADER is required. Usage: make KERNEL_BOOTLOADER=<bootloader_folder_name>)
endif

BOOTLOADER_MAKEFILE := $(PROJECT_ROOT)boot/$(ARCH)/$(KERNEL_BOOTLOADER)/Makefile
ifeq ($(wildcard $(BOOTLOADER_MAKEFILE)),)
$(error Unsupported KERNEL_BOOTLOADER '$(KERNEL_BOOTLOADER)' for ARCH '$(ARCH)')
endif

CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld
OBJCOPY := $(CROSS_COMPILE)objcopy
OBJDUMP := $(CROSS_COMPILE)objdump

COMMON_FLAGS := 				\
	-ffreestanding 				\
	-fno-stack-protector 	\
	-fno-pie 						\
	$(ARCH_FLAGS)

CFLAGS := $(COMMON_FLAGS) -std=gnu11
ASFLAGS := $(COMMON_FLAGS)

LDFLAGS := 	\
	-nostdlib \
	-static
