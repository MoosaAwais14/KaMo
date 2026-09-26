.PHONY: all arch generic compile_commands
.DELETE_ON_ERROR:

OUT := $(PROJECT_OUT)/kernel

LINKER := kernel.ld

TARGET := $(PROJECT_OUT)/kernel.o

INCLUDE_FLAGS := \
	-I$(PROJECT_DIR)/kernel/include \
	-I$(PROJECT_DIR)/kernel/arch/$(ARCH)/include

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

$(TARGET): arch generic
	mkdir -p $(dir $@)
	$(LD) -r -T $(LINKER) \
		$(shell find $(OUT) -type f -name '*.o' | LC_ALL=C sort) \
		-o $@

arch:
	$(MAKE) -C arch/$(ARCH) CFLAGS="$(CFLAGS)" ASFLAGS="$(ASFLAGS)" OUT="$(OUT)"

generic:
	$(MAKE) -C kernel CFLAGS="$(CFLAGS)" ASFLAGS="$(ASFLAGS)" OUT="$(OUT)"
	$(MAKE) -C mm CFLAGS="$(CFLAGS)" ASFLAGS="$(ASFLAGS)" OUT="$(OUT)"

compile_commands:
	printf '%s\n' $(COMMON_FLAGS) > compile_flags.txt
