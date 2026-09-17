include config.mk

PROJECT_DIR := $(CURDIR)

export PROJECT_DIR
export ARCH
export CC
export LD
export OBJCOPY
export OBJDUMP
export COMMON_FLAGS
export CFLAGS
export ASFLAGS
export LDFLAGS

ifndef KERNEL_BOOTLOADER
$(error KERNEL_BOOTLOADER is required. Usage: make KERNEL_BOOTLOADER=<bootloader_folder_name>)
endif

export KERNEL_BOOTLOADER

.PHONY: all image kamo kernel userspace clean compile_commands

all: userspace kamo kernel

image: userspace kamo kernel
	$(MAKE) -C image

kamo:
	$(MAKE) -C kamo

kernel:
	$(MAKE) -C kernel

userspace:
	$(MAKE) -C userspace

compile_commands:
	$(MAKE) clean
	bear -- $(MAKE) ARCH=$(ARCH) KERNEL_BOOTLOADER=$(KERNEL_BOOTLOADER) all
	$(MAKE) clean

	$(MAKE) -C kamo compile_commands
	$(MAKE) -C kernel compile_commands
	$(MAKE) -C userspace compile_commands

clean:
	$(MAKE) -C image clean
	$(MAKE) -C kamo clean
	$(MAKE) -C kernel clean
	$(MAKE) -C userspace clean

