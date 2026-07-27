.PHONY: all GRUB2 clean

BUILD := $(CURDIR)/build
export BUILD

LINKER := kamo.ld

TARGET := $(BUILD)/kamo.o

OBJECTS = \
	$(wildcard $(BUILD)/*.o)

all: $(TARGET)

$(TARGET): GRUB2
	mkdir -p $(dir $@)
	$(LD) -r -T $(LINKER) $(OBJECTS) -o $@

GRUB2:
	$(MAKE) -C GRUB2

clean:
	rm -rf $(BUILD)
