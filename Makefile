include config.mk

export ARCH
export KERNEL_BOOTLOADER
export CC
export LD
export OBJCOPY
export OBJDUMP

export COMMON_FLAGS
export CFLAGS
export ASFLAGS
export LDFLAGS

PROJECT_DIR := $(CURDIR)
export PROJECT_DIR

PROJECT_OUT := $(PROJECT_DIR)/out/$(ARCH)/$(KERNEL_BOOTLOADER)
export PROJECT_OUT

.PHONY: all kamo boot kernel userspace build package clean compile_commands

all: boot kernel userspace build

kamo: all

build: boot kernel
	$(MAKE) -C build

boot:
	$(MAKE) -C boot

kernel:
	$(MAKE) -C kernel

userspace:
	$(MAKE) -C userspace

package: boot kernel
	$(MAKE) -C build package

compile_commands:
	$(MAKE) -C boot compile_commands
	$(MAKE) -C kernel compile_commands

clean:
	rm -rf $(PROJECT_DIR)/out
