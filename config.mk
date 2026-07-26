ARCH ?= x86

ifeq ($(ARCH),x86)
TARGET := i686-elf
endif

CC := $(TARGET)-gcc


