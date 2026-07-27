include config.mk

PROJECT_DIR := $(CURDIR)

export PROJECT_DIR
export ARCH
export CC
export LD
export OBJCOPY
export OBJDUMP
export CFLAGS
export ASFLAGS
export LDFLAGS

.PHONY: all image kamo kernel userspace clean

all: image userspace

image: kamo kernel
	$(MAKE) -C image

kamo:
	$(MAKE) -C kamo

kernel:
	$(MAKE) -C kernel

userspace:
	$(MAKE) -C userspace

clean:
	$(MAKE) -C image clean
	$(MAKE) -C kamo clean
	$(MAKE) -C kernel clean
	$(MAKE) -C userspace clean
