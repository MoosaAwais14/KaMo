.PHONY: all arch generic drivers clean

all: arch generic drivers

arch:
	$(MAKE) -C arch

generic:
	$(MAKE) -C kernel

drivers:
	$(MAKE) -C drivers

clean:
	$(MAKE) -C arch clean
	$(MAKE) -C kernel clean
	$(MAKE) -C drivers clean
