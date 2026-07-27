ARCH ?= x86

ifeq ($(ARCH),x86)
TARGET := i686-elf
endif

CC := $(TARGET)-gcc
LD := $(TARGET)-ld
OBJCOPY := $(TARGET)-objcopy
OBJDUMP := $(TARGET)-objdump

COMMON_FLAGS := \
	-ffreestanding \
	-fno-stack-protector \
	-fno-pie \
	-m32

CFLAGS := $(COMMON_FLAGS) -std=gnu11
ASFLAGS := $(COMMON_FLAGS)

LDFLAGS := \
	-nostdlib \
	-static
