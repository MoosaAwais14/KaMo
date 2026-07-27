.PHONY: all clean

BUILD := build

SOURCES := \
	GRUB2/header.S \
	GRUB2/entry.S \
	GRUB2/main.c

OBJECTS := $(SOURCES:%=$(BUILD)/%.o)

LINKER := kamo.ld

TARGET := $(BUILD)/kamo.o

all: $(TARGET)

$(TARGET): $(OBJECTS)
	mkdir -p $(dir $@)
	$(LD) -r -T $(LINKER) $^ -o $@

$(BUILD)/%.S.o: %.S
	mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) -c $< -o $@

$(BUILD)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD)
