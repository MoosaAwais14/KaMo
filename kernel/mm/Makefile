.PHONY: all

SOURCES := 		\

OBJECTS := $(SOURCES:%=$(BUILD)/%.o)

all: $(OBJECTS)

$(BUILD)/%.S.o: %.S
	mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) -c $< -o $@

$(BUILD)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@
