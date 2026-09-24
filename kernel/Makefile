.PHONY: all arch generic drivers compile_commands

OUT := $(PROJECT_OUT)/kernel

LINKER := kernel.ld

TARGET := $(PROJECT_OUT)/kernel.o

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
	$(shell find $(OUT) -type f -name '*.o')

all: arch generic $(TARGET)

$(TARGET): 
	mkdir -p $(dir $@)
	$(LD) -r -T $(LINKER) 												\
		$(shell find $(OUT) -type f -name '*.o')	\
		-o $@

arch:
	$(MAKE) -C arch/$(ARCH) CFLAGS="$(CFLAGS)" ASFLAGS="$(ASFLAGS)" OUT="$(OUT)"

generic:
	$(MAKE) -C kernel	CFLAGS="$(CFLAGS)" ASFLAGS="$(ASFLAGS)" OUT="$(OUT)"
	$(MAKE) -C mm	CFLAGS="$(CFLAGS)" ASFLAGS="$(ASFLAGS)" OUT="$(OUT)"

compile_commands:
	printf '%s\n' $(COMMON_FLAGS) > compile_flags.txt
