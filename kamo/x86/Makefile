.PHONY: all $(KERNEL_BOOTLOADER) clean

BUILD := $(CURDIR)/build

LINKER := kamo.ld

TARGET := $(BUILD)/kamo.o

OBJECTS = \
	$(wildcard $(BUILD)/*.o)

all: $(TARGET)

$(TARGET): $(KERNEL_BOOTLOADER)
	mkdir -p $(dir $@)
	$(LD) -r -T $(LINKER) $(OBJECTS) -o $@

$(KERNEL_BOOTLOADER):
	$(MAKE) -C $(KERNEL_BOOTLOADER) CFLAGS="$(CFLAGS) -I$(CURDIR)/include -I$(PROJECT_DIR)/kernel/include" BUILD="$(BUILD)"

clean:
	rm -rf $(BUILD)
