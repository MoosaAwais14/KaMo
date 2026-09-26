.PHONY: all
.DELETE_ON_ERROR:

SOURCES := \
	entry.S \
	main.c \
	multiboot2_info.c \
	uart.c

OBJECTS := $(SOURCES:%=$(OUT)/%.o)

all: $(OBJECTS)

$(OUT)/%.S.o: %.S
	mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) -MMD -MP -MF $(@:.o=.d) -c $< -o $@

$(OUT)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -MF $(@:.o=.d) -c $< -o $@

-include $(OBJECTS:.o=.d)


