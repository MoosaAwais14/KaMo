include config.mk

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

PROJECT_DIR := $(CURDIR)
export PROJECT_DIR

PROJECT_OUT := $(PROJECT_DIR)/out
export PROJECT_OUT

.PHONY: all kamo boot kernel userspace package clean compile_commands

all: boot kernel userspace

kamo: all
	$(MAKE) -C build

boot:
	$(MAKE) -C boot

kernel:
	$(MAKE) -C kernel

userspace:
	$(MAKE) -C userspace

package:
	$(MAKE) -C build package

compile_commands:
	$(MAKE) -C build compile_commands

clean:
	rm -rf $(PROJECT_OUT)
