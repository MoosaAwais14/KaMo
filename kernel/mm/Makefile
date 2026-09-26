.PHONY: all
.DELETE_ON_ERROR:

ifndef OUT
$(error OUT is required. Usage: make ... OUT=<dir>)
endif

SOURCES := \
	memblock.c

OBJECTS := $(SOURCES:%=$(OUT)/%.o)

all: $(OBJECTS)

$(OUT)/%.S.o: %.S
	mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) -MMD -MP -MF $(@:.o=.d) -c $< -o $@

$(OUT)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -MF $(@:.o=.d) -c $< -o $@

-include $(OBJECTS:.o=.d)
