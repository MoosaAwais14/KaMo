.PHONY: all
.DELETE_ON_ERROR:

OUT := $(PROJECT_OUT)/link

KAMO_LD := $(PROJECT_DIR)/kernel/arch/$(ARCH)/kernel.ld
KAMO_ELF := $(PROJECT_OUT)/kamo.elf
KAMO_MAP := $(OUT)/kamo.map

BOOT_OBJ := $(PROJECT_OUT)/boot.o
BOOT_LINK_OBJ := $(OUT)/boot.link.o
KERNEL_OBJ := $(PROJECT_OUT)/kernel.o

all: $(KAMO_ELF)

$(KAMO_ELF): $(BOOT_LINK_OBJ) $(KERNEL_OBJ)
	mkdir -p $(dir $@)
	$(LD) \
		-T $(KAMO_LD) \
		-Map $(KAMO_MAP) \
		$(BOOT_LINK_OBJ) \
		$(KERNEL_OBJ) \
		-o $@

$(BOOT_LINK_OBJ): $(BOOT_OBJ)
	mkdir -p $(dir $@)
	$(OBJCOPY) \
		--rename-section .text=.boot.text \
		--rename-section .rodata=.boot.rodata \
		--rename-section .data=.boot.data \
		--rename-section .bss=.boot.bss \
		$< $@

