.PHONY: all

STAGING_DIR := $(PROJECT_OUT)/staging
STAGING_BOOT_DIR := $(STAGING_DIR)/boot

all: stage
	
stage:
	rm -rf $(STAGING_DIR)
	mkdir -p $(STAGING_DIR)
	mkdir -p $(STAGING_BOOT_DIR)

	$(MAKE) -C $(ARCH)/$(KERNEL_BOOTLOADER) BOOT_DIR=$(STAGING_BOOT_DIR)


