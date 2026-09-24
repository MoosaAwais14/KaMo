.PHONY: all

SOURCES := 					\
	entry.S  					\
	main.c 						\
	multiboot2_info.c	\
	uart.c						\

OBJECTS := $(SOURCES:%=$(OUT)/%.o)

all: $(OBJECTS)

$(OUT)/%.S.o: %.S
	mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) -c $< -o $@

$(OUT)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@


