.PHONY: all $(KERNEL_BOOTLOADER) clean

BUILD := $(CURDIR)/build
export BUILD

LINKER := kamo.ld

TARGET := $(BUILD)/kamo.o

OBJECTS = \
	$(wildcard $(BUILD)/*.o)

all: $(TARGET)

$(TARGET): $(KERNEL_BOOTLOADER)
	mkdir -p $(dir $@)
	$(LD) -r -T $(LINKER) $(OBJECTS) -o $@

$(KERNEL_BOOTLOADER):
	$(MAKE) -C $(KERNEL_BOOTLOADER)

clean:
	rm -rf $(BUILD)
