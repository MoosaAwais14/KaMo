.PHONY: all FORCE
.DELETE_ON_ERROR:

ifndef BOOT_DIR
$(error BOOT_DIR is required; invoke packaging through the top-level make package target)
endif

KAMO_ELF := $(PROJECT_OUT)/kamo.elf
GRUB_CONFIG := $(PROJECT_DIR)/config/grub/grub.cfg
GRUB_DIR := $(BOOT_DIR)/grub
STAGING_DIR := $(abspath $(BOOT_DIR)/..)
KAMO_ISO := $(PROJECT_OUT)/kamo.iso


all: $(KAMO_ISO)

FORCE:

$(KAMO_ISO): $(KAMO_ELF) $(GRUB_CONFIG) FORCE
	if ! grub-file --is-x86-multiboot2 "$(KAMO_ELF)"; then \
		echo "$(KAMO_ELF) is not a valid Multiboot2 kernel"; \
		exit 1; \
	fi; \

	mkdir -p "$(GRUB_DIR)"
	cp "$(KAMO_ELF)" "$(BOOT_DIR)/kimage.elf"
	cp "$(GRUB_CONFIG)" "$(GRUB_DIR)/grub.cfg"
	grub-mkrescue -o "$(KAMO_ISO)" "$(STAGING_DIR)"
	@echo "Created $(KAMO_ISO)"




