.PHONY: all clean

BUILD := build

K_DEPEND_LD := kimage.ld.in
K_TARGET_LD := $(BUILD)/kimage.ld.out
K_TARGET := $(BUILD)/kimage.elf
K_MAP := $(BUILD)/kimage.map

KAMO_OBJ := $(PROJECT_DIR)/kamo/build/kamo.o
KAMO_LINK_OBJ := $(BUILD)/kamo.link.o
KERNEL_OBJ := $(PROJECT_DIR)/kernel/build/kernel.o

include $(PROJECT_DIR)/kamo/$(ARCH)/$(KERNEL_BOOTLOADER)/package_boot.mk

all: package_boot

package_boot: $(PACKAGE_TARGET)

$(K_TARGET): $(K_TARGET_LD) $(KAMO_LINK_OBJ) $(KERNEL_OBJ)
	mkdir -p $(dir $@)
	$(LD) \
		-T $(K_TARGET_LD) \
		-Map $(K_MAP) \
		$(KAMO_LINK_OBJ) \
		$(KERNEL_OBJ) \
		-o $@

$(K_TARGET_LD): $(K_DEPEND_LD)
	mkdir -p $(BUILD)
	sed \
		-e 's|@ARCH@|$(ARCH)|g' \
		-e 's|@PROJECT_DIR@|$(PROJECT_DIR)|g' \
		$< > $@

$(KAMO_LINK_OBJ): $(KAMO_OBJ)
	mkdir -p $(dir $@)
	$(OBJCOPY) \
		--rename-section .text=.kamo.text \
		--rename-section .rodata=.kamo.rodata \
		--rename-section .data=.kamo.data \
		--rename-section .bss=.kamo.bss \
		$< $@

clean:
	rm -rf $(BUILD)
