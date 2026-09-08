GRUB2_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
GRUB_CFG := $(GRUB2_DIR)grub.cfg

PACKAGE_TARGET := $(BUILD)/image.img

IMAGE_SIZE := 64M

.PHONY: package_boot

package_boot: $(PACKAGE_TARGET)

$(PACKAGE_TARGET): $(K_TARGET) $(GRUB_CFG)
	@set -e; \
	echo "Checking Multiboot2 kernel..."; \
	if ! grub-file --is-x86-multiboot2 "$(K_TARGET)"; then \
		echo "$(K_TARGET) is not a valid Multiboot2 kernel"; \
		exit 1; \
	fi; \
	\
	echo "Creating $(PACKAGE_TARGET)..."; \
	mkdir -p "$(dir $@)"; \
	rm -f "$@"; \
	truncate -s 64M "$@"; \
	\
	echo "Creating partition table..."; \
	sudo parted -s "$@" \
		mklabel msdos \
		mkpart primary fat32 1MiB 100% \
		set 1 boot on; \
	\
	echo "Attaching image..."; \
	LOOP=$$(sudo losetup --find --show --partscan "$@"); \
	echo "Loop device: $$LOOP"; \
	\
	PART="$${LOOP}p1"; \
	sudo mkfs.fat -F 32 "$$PART"; \
	\
	MOUNT=$$(mktemp -d); \
	trap 'sudo umount "$$MOUNT" 2>/dev/null || true; rmdir "$$MOUNT" 2>/dev/null || true; sudo losetup -d "$$LOOP" 2>/dev/null || true' EXIT; \
	\
	sudo mount "$$PART" "$$MOUNT"; \
	\
	sudo mkdir -p "$$MOUNT/boot/grub"; \
	sudo cp "$(K_TARGET)" "$$MOUNT/boot/kimage.elf"; \
	sudo cp "$(GRUB_CFG)" "$$MOUNT/boot/grub/grub.cfg"; \
	\
	sudo grub-install \
		--target=i386-pc \
		--boot-directory="$$MOUNT/boot" \
		--no-floppy \
		"$$LOOP"; \
	\
	sudo umount "$$MOUNT"; \
	sudo losetup -d "$$LOOP"; \
	\
	echo "Created $@"

