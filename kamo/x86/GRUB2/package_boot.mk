GRUB2_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
GRUB_CFG := $(GRUB2_DIR)/grub.cfg

.PHONY: package_boot

package_boot: $(PACKAGE_TARGET)

$(PACKAGE_TARGET): $(eval SHELL:=/bin/bash) $(TARGET)
	@if ! grub-file --is-x86-multiboot2 "$(TARGET)"; then \
		echo "$(TARGET) is not a valid Multiboot2 kernel"; \
		exit 1; \
	fi

	
