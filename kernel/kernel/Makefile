.PHONY: all

ifndef OUT
$(error OUT is required. Usage: make ... OUT=<dir>)
endif

SOURCES := 		\
	main.c			\
	cpu.c				\
	interrupt.c	\
	irq.c				\

OBJECTS := $(SOURCES:%=$(OUT)/%.o)

all: $(OBJECTS)

$(OUT)/%.S.o: %.S
	mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) -c $< -o $@

$(OUT)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@
