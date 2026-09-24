.PHONY: all compile_commands

ifndef OUT 
$(error OUT is required. Usage: make ... OUT=<dir>)
endif

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

OBJECTS := $(SOURCES:%=$(OUT)/%.o)

all: $(OBJECTS)

$(OUT)/%.S.o: %.S
	mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) -c $< -o $@

$(OUT)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

compile_commands:
	printf '%s\n' $(COMMON_FLAGS) > compile_flags.txt
