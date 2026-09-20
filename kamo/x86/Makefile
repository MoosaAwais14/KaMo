.PHONY: all $(KERNEL_BOOTLOADER) clean compile_commands

LINKER := kamo.ld

TARGET := $(BUILD)/kamo.o

INCLUDE_FLAGS := 									\
	-I$(CURDIR)/include 						\
	-I$(PROJECT_DIR)/kernel/include 

COMMON_FLAGS := 		\
	$(COMMON_FLAGS) 	\
	$(INCLUDE_FLAGS)

CFLAGS := 					\
	$(CFLAGS) 				\
	$(INCLUDE_FLAGS)

ASFLAGS :=					\
	$(ASFLAGS) 				\
	$(INCLUDE_FLAGS)

OBJECTS := 																		\
	$(shell find $(BUILD) -type f -name '*.o')

all: $(TARGET)

$(TARGET): $(KERNEL_BOOTLOADER)
	mkdir -p $(dir $@)
	$(LD) -r -T $(LINKER) 												\
		$(shell find $(BUILD) -type f -name '*.o')	\
		-o $@

$(KERNEL_BOOTLOADER):
	$(MAKE) -C $(KERNEL_BOOTLOADER) CFLAGS="$(CFLAGS)" ASFLAGS="" BUILD="$(BUILD)"

compile_commands:
		printf '%s\n' $(COMMON_FLAGS) > compile_flags.txt
