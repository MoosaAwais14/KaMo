.PHONY: all arch compiled_commands

OUT := $(PROJECT_OUT)/boot

all: arch 

arch:
	$(MAKE) -C $(ARCH) OUT="$(OUT)"

compile_commands:
	$(MAKE) -C $(ARCH) OUT="$(OUT)" compile_commands
