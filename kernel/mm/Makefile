.PHONY: all
.DELETE_ON_ERROR:

ifndef OUT
$(error OUT is required. Usage: make ... OUT=<dir>)
endif

SOURCES := 			\
	earlybump.c		\
	memblock.c		\
	frame_alloc.c	\
	mmu.c			\
	vma.c

OBJECTS := $(SOURCES:%=$(OUT)/%.o)

all: $(OBJECTS)

$(OUT)/%.S.o: %.S
	mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) -MMD -MP -MF $(@:.o=.d) -c $< -o $@

$(OUT)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -MF $(@:.o=.d) -c $< -o $@

-include $(OBJECTS:.o=.d)
