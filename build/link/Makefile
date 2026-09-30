.PHONY: all
.DELETE_ON_ERROR:

OUT := $(PROJECT_OUT)/link

KAMO_LD_S := $(PROJECT_DIR)/kernel/arch/$(ARCH)/vmkernel.ld.S
KAMO_LD   := $(OUT)/vmkernel.ld
KAMO_LD_DEP := $(KAMO_LD).d

LINKER_CPPFLAGS := \
	-DLINKER_SCRIPT \
	-I$(PROJECT_DIR)/kernel/include \
	-I$(PROJECT_DIR)/kernel/arch/$(ARCH)/include

KAMO_ELF := $(PROJECT_OUT)/kamo.elf
KAMO_MAP := $(OUT)/kamo.map

BOOT_OBJ      := $(PROJECT_OUT)/boot.o
BOOT_LINK_OBJ := $(OUT)/boot.link.o
KERNEL_OBJ    := $(PROJECT_OUT)/kernel.o

all: $(KAMO_ELF)

$(KAMO_ELF): $(BOOT_LINK_OBJ) $(KERNEL_OBJ) $(KAMO_LD)
	mkdir -p $(dir $@)
	$(LD) \
		-T $(KAMO_LD) \
		-Map $(KAMO_MAP) \
		$(BOOT_LINK_OBJ) \
		$(KERNEL_OBJ) \
		-o $@

$(KAMO_LD): $(KAMO_LD_S)
	mkdir -p $(dir $@)
	$(CC) \
		-E \
		-P \
		-x c \
		-MMD \
		-MF $(KAMO_LD_DEP) \
		-MT $@ \
		$(LINKER_CPPFLAGS) \
		$< \
		-o $@

$(BOOT_LINK_OBJ): $(BOOT_OBJ)
	mkdir -p $(dir $@)
	$(OBJCOPY) \
		--rename-section .text=.boot.text \
		--rename-section .rodata=.boot.rodata \
		--rename-section .data=.boot.data \
		--rename-section .bss=.boot.bss \
		$< $@

-include $(KAMO_LD_DEP)

