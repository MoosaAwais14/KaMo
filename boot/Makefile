.PHONY: all arch compile_commands

OUT := $(PROJECT_OUT)/boot

all: arch

arch:
	$(MAKE) -C $(ARCH) OUT="$(OUT)"

compile_commands:
	$(MAKE) -C $(ARCH) OUT="$(OUT)" compile_commands
