.PHONY: all clean

BUILD := build

SOURCES := 								\
	kernel/setup.c					\
	kernel/cpu/cpu.c				\
	kernel/cpu/gdt.c				\
	kernel/cpu/idt.S 				\
	kernel/cpu/idt.c				\
	kernel/interrupt/irq.c	\

OBJECTS := $(SOURCES:%=$(BUILD)/%.o)

all: $(OBJECTS)

$(BUILD)/%.S.o: %.S
	mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) -c $< -o $@

$(BUILD)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD)
