.PHONY: all arch clean compiled_commands

BUILD := $(CURDIR)/build

all: arch 

arch:
	$(MAKE) -C $(ARCH) BUILD="$(BUILD)"

compile_commands:
	$(MAKE) -C $(ARCH) BUILD="$(BUILD)" compile_commands

clean:
	rm -rf $(BUILD)

