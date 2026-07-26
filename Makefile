include config.mk

PROJECT_DIR := $(CURDIR)

export PROJECT_DIR
export ARCH
export CC

.PHONY: all kamo kernel userspace clean

all: kamo kernel userspace

kamo:
	$(MAKE) -C kamo

kernel:
	$(MAKE) -C kernel

userspace:
	$(MAKE) -C userspace

clean:
	$(MAKE) -C kamo clean
	$(MAKE) -C kernel clean
	$(MAKE) -C userspace clean
