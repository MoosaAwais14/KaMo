ifndef ARCH
$(error ARCH is required. Usage: make ARCH=<architecture>)
endif

ifeq ($(ARCH),x86)
TARGET := i686-elf
endif

CC := $(TARGET)-gcc
LD := $(TARGET)-ld
OBJCOPY := $(TARGET)-objcopy
OBJDUMP := $(TARGET)-objdump

ifeq ($(ARCH),x86)
COMMON_FLAGS := 				\
	-ffreestanding 				\
	-fno-stack-protector 	\
	-fno-pie							\
	-m32
endif

CFLAGS := $(COMMON_FLAGS) -std=gnu11
ASFLAGS := $(COMMON_FLAGS)

LDFLAGS := 	\
	-nostdlib \
	-static
