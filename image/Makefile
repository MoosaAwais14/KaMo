.PHONY: all clean

BUILD := build

TARGET_LD := $(BUILD)/image.ld.out
TARGET := $(BUILD)/image.elf
MAP := $(BUILD)/image.map

PACKAGE_TARGET := $(BUILD)/image.img

KAMO_OBJ := $(PROJECT_DIR)/kamo/$(ARCH)/build/kamo.o
KAMO_LINK_OBJ := $(BUILD)/kamo.link.o
KERNEL_OBJ := $(PROJECT_DIR)/kernel/build/kernel.o

include $(PROJECT_DIR)/kamo/$(ARCH)/$(KERNEL_BOOTLOADER)/package_boot.mk

all: package_boot

$(TARGET): $(TARGET_LD) $(KAMO_LINK_OBJ) $(KERNEL_OBJ)
	mkdir -p $(dir $@)
	$(LD) \
		-T $(TARGET_LD) \
		-Map $(MAP) \
		$(KAMO_LINK_OBJ) \
		$(KERNEL_OBJ) \
		-o $@

$(TARGET_LD): image.ld.in
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
