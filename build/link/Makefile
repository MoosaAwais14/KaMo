.PHONY: all

OUT := $(PROJECT_OUT)/link

KAMO_LD_IN  := kamo.ld.in
KAMO_LD     := $(OUT)/kamo.ld
KAMO_ELF    := $(PROJECT_OUT)/kamo.elf
KAMO_MAP    := $(OUT)/kamo.map

BOOT_OBJ 			:= $(PROJECT_OUT)/boot.o
BOOT_LINK_OBJ := $(OUT)/boot.link.o
KERNEL_OBJ 		:= $(PROJECT_OUT)/kernel.o

all: $(KAMO_ELF)

$(KAMO_ELF): $(KAMO_LD) $(BOOT_LINK_OBJ) $(KERNEL_OBJ)
	mkdir -p $(dir $@)
	$(LD) \
		-T $(KAMO_LD) \
		-Map $(KAMO_MAP) \
		$(BOOT_LINK_OBJ) \
		$(KERNEL_OBJ) \
		-o $@

$(KAMO_LD): $(KAMO_LD_IN)
	mkdir -p $(OUT)
	sed \
		-e 's|@ARCH@|$(ARCH)|g' \
		-e 's|@PROJECT_DIR@|$(PROJECT_DIR)|g' \
		$< > $@

$(BOOT_LINK_OBJ): $(BOOT_OBJ)
	mkdir -p $(dir $@)
	$(OBJCOPY) \
		--rename-section .text=.boot.text \
		--rename-section .rodata=.boot.rodata \
		--rename-section .data=.boot.data \
		--rename-section .bss=.boot.bss \
		$< $@

