.PHONY: all $(KERNEL_BOOTLOADER) clean compile_commands

BUILD := $(CURDIR)/build

LINKER := kamo.ld

TARGET := $(BUILD)/kamo.o

COMMON_FLAGS := 	 								\
	$(COMMON_FLAGS) 								\
	-I$(CURDIR)/include 						\
	-I$(PROJECT_DIR)/kernel/include \

OBJECTS := \
	$(shell find $(BUILD) -type f -name '*.o')

all: $(TARGET)

$(TARGET): $(KERNEL_BOOTLOADER)
	mkdir -p $(dir $@)
	$(LD) -r -T $(LINKER) 												\
		$(shell find $(BUILD) -type f -name '*.o')	\
		-o $@

$(KERNEL_BOOTLOADER):
	$(MAKE) -C $(KERNEL_BOOTLOADER) CFLAGS="$(CFLAGS) -I$(CURDIR)/include -I$(PROJECT_DIR)/kernel/include" ASFLAGS="$(ASFLAGS) -I$(CURDIR)/include -I$(PROJECT_DIR)/kernel/include" BUILD="$(BUILD)"

compile_commands:
		printf '%s\n' $(COMMON_FLAGS) > compile_flags.txt

clean:
	rm -rf $(BUILD)
