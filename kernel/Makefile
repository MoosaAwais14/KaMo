.PHONY: all arch generic drivers clean compile_commands

BUILD := build

LINKER := kernel.ld

TARGET := $(BUILD)/kernel.o

all: arch generic drivers $(TARGET)

$(TARGET): 
	mkdir -p $(dir $@)
	$(LD) -r -T $(LINKER)																															\
		$(shell find arch/$(ARCH)/build kernel/build drivers/build -type f -name '*.o') \
		-o $@

arch:
	$(MAKE) -C arch/$(ARCH)	CFLAGS="$(CFLAGS) -I$(PROJECT_DIR)/kernel/include -I$(PROJECT_DIR)/kernel/arch/$(ARCH)/include" ASFLAGS="$(ASFLAGS) -I$(PROJECT_DIR)/kernel/include -I$(PROJECT_DIR)/kernel/arch/$(ARCH)/include"

generic:
	$(MAKE) -C kernel	CFLAGS="$(CFLAGS) -I$(PROJECT_DIR)/kernel/include -I$(PROJECT_DIR)/kernel/kernel/include -I$(PROJECT_DIR)/kernel/arch/$(ARCH)/include" ASFLAGS="$(ASFLAGS) -I$(PROJECT_DIR)/kernel/include -I$(PROJECT_DIR)/kernel/kernel/include -I$(PROJECT_DIR)/kernel/arch/$(ARCH)/include"

drivers:
	$(MAKE) -C drivers CFLAGS="$(CFLAGS) -I$(PROJECT_DIR)/kernel/include" ASFLAGS="$(ASFLAGS) -I$(PROJECT_DIR)/kernel/include"

compile_commands:
	$(MAKE) -C arch/$(ARCH)	COMMON_FLAGS="$(COMMON_FLAGS) -I$(PROJECT_DIR)/kernel/include -I$(PROJECT_DIR)/kernel/arch/$(ARCH)/include" compile_commands
	$(MAKE) -C arch/$(ARCH)	COMMON_FLAGS="$(COMMON_FLAGS) -I$(PROJECT_DIR)/kernel/include -I$(PROJECT_DIR)/kernel/arch/$(ARCH)/include" compile_commands
	$(MAKE) -C arch/$(ARCH)	COMMON_FLAGS="$(COMMON_FLAGS) -I$(PROJECT_DIR)/kernel/include -I$(PROJECT_DIR)/kernel/arch/$(ARCH)/include" compile_commands

clean:
	$(MAKE) -C arch/$(ARCH) clean
	$(MAKE) -C kernel clean
	$(MAKE) -C drivers clean
	rm -rf $(BUILD)
