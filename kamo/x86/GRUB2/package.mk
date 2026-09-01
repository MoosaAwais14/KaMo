GRUB2_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
GRUB_CFG := $(GRUB2_DIR)/grub.cfg

.PHONY: package

package: $(PACKAGE_TARGET)

$(PACKAGE_TARGET): $(TARGET)
	@echo "Packaging GRUB2 image..."

