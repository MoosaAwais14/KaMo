.PHONY: all $(KERNEL_BOOTLOADER) compile_commands
.DELETE_ON_ERROR:

ifndef OUT
$(error OUT is required. Usage: make ... OUT=<dir>)
endif

LINKER := boot.ld

TARGET := $(PROJECT_OUT)/boot.o

INCLUDE_FLAGS := \
	-I$(CURDIR)/include \
	-I$(PROJECT_DIR)/kernel/include

COMMON_FLAGS := \
	$(COMMON_FLAGS) \
	$(INCLUDE_FLAGS)

CFLAGS := \
	$(CFLAGS) \
	$(INCLUDE_FLAGS)

ASFLAGS := \
	$(ASFLAGS) \
	$(INCLUDE_FLAGS)

all: $(TARGET)

$(TARGET): $(KERNEL_BOOTLOADER)
	mkdir -p $(dir $@)
	$(LD) -r -T $(LINKER) \
		$(shell find $(OUT) -type f -name '*.o' | LC_ALL=C sort) \
		-o $@

$(KERNEL_BOOTLOADER):
	$(MAKE) -C $(KERNEL_BOOTLOADER) CFLAGS="$(CFLAGS)" ASFLAGS="" OUT="$(OUT)"

compile_commands:
	printf '%s\n' $(COMMON_FLAGS) > compile_flags.txt
