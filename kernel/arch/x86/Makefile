.PHONY: all compile_commands

SOURCES := 						\
	kernel/setup.c			\
	kernel/cpu.c				\
	kernel/gdt.c				\
	kernel/idt.S 				\
	kernel/idt.c				\
	kernel/irq.c				\
	kernel/irqchip.c		\
	kernel/i8259.c			\
	kernel/exception.c	\
	sync/spinlock.c			\

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
