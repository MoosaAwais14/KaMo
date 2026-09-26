.PHONY: all stage

STAGING_DIR := $(PROJECT_OUT)/staging
STAGING_BOOT_DIR := $(STAGING_DIR)/boot
PACKAGE_DIR := $(ARCH)/$(KERNEL_BOOTLOADER)

ifeq ($(wildcard $(PACKAGE_DIR)/Makefile),)
$(error No packaging rules found for ARCH='$(ARCH)' KERNEL_BOOTLOADER='$(KERNEL_BOOTLOADER)')
endif

all: stage

stage:
	rm -rf $(STAGING_DIR)
	mkdir -p $(STAGING_DIR)
	mkdir -p $(STAGING_BOOT_DIR)

	$(MAKE) -C $(PACKAGE_DIR) BOOT_DIR="$(STAGING_BOOT_DIR)"


