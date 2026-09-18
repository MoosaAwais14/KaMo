.PHONY: all clean compile_commands

BUILD := build

SOURCES := 										\
	kernel/setup.c							\
	kernel/cpu/cpu.c						\
	kernel/cpu/gdt.c						\
	kernel/cpu/idt.S 						\
	kernel/cpu/idt.c						\
	kernel/interrupt/irq.c			\
	kernel/interrupt/irqchip.c	\
	kernel/i8259.c							\

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
