.PHONY: all arch generic drivers clean compile_commands

BUILD := $(CURDIR)/build

LINKER := kernel.ld

TARGET := $(BUILD)/kernel.o

INCLUDE_FLAGS :=																\
	-I$(PROJECT_DIR)/kernel/include 							\
	-I$(PROJECT_DIR)/kernel/arch/$(ARCH)/include

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

all: arch generic $(TARGET)

$(TARGET): 
	mkdir -p $(dir $@)
	$(LD) -r -T $(LINKER) 												\
		$(shell find $(BUILD) -type f -name '*.o')	\
		-o $@

arch:
	$(MAKE) -C arch/$(ARCH) CFLAGS="$(CFLAGS)" ASFLAGS="$(ASFLAGS)" BUILD="$(BUILD)"

generic:
	$(MAKE) -C kernel	CFLAGS="$(CFLAGS)" ASFLAGS="$(ASFLAGS)" BUILD="$(BUILD)"
	$(MAKE) -C mm	CFLAGS="$(CFLAGS)" ASFLAGS="$(ASFLAGS)" BUILD="$(BUILD)"
	$(MAKE) -C drivers CFLAGS="$(CFLAGS)" ASFLAGS="$(ASFLAGS)" BUILD="$(BUILD)"

compile_commands:
	printf '%s\n' $(COMMON_FLAGS) > compile_flags.txt

clean:
	rm -rf $(BUILD)
