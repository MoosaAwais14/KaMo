.PHONY: all arch generic drivers clean

BUILD := build

LINKER := kernel.ld

TARGET := $(BUILD)/kernel.o

OBJECTS = \
	$(wildcard arch/$(ARCH)/build/*.o) \
	$(wildcard kernel/build/*.o) \
	$(wildcard drivers/build/*.o)

all: arch generic drivers $(TARGET)

$(TARGET): 
	mkdir -p $(dir $@)
	$(LD) -r -T $(LINKER) $(OBJECTS) -o $@

arch:
	$(MAKE) -C arch/$(ARCH)	CFLAGS="$(CFLAGS) -I$(PROJECT_DIR)/kernel/include -I$(PROJECT_DIR)/kernel/arch/$(ARCH)/include"

generic:
	$(MAKE) -C kernel	CFLAGS="$(CFLAGS) -I$(PROJECT_DIR)/kernel/include -I$(PROJECT_DIR)/kernel/kernel/include -I$(PROJECT_DIR)/kernel/arch/$(ARCH)/include"

drivers:
	$(MAKE) -C drivers CFLAGS="$(CFLAGS) -I$(PROJECT_DIR)/kernel/include"

clean:
	$(MAKE) -C arch/$(ARCH) clean
	$(MAKE) -C kernel clean
	$(MAKE) -C drivers clean
	rm -rf $(BUILD)
