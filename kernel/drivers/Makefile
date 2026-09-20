.PHONY: all clean compile_commands

BUILD := build

SOURCES := \
	
OBJECTS := $(SOURCES:%=$(BUILD)/%.o)

all: $(OBJECTS)

$(BUILD)/%.S.o: %.S
	mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) -c $< -o $@

$(BUILD)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

compile_commands:
	printf '%s\n' $(COMMON_FLAGS) > compile_flags.txt

clean:
	rm -rf $(BUILD)
