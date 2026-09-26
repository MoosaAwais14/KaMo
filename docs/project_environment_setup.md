# Getting Started

You'll need GNU Make and the `i686-elf` cross-toolchain. To make the GRUB ISO,
you'll also need `grub-file`, `grub-mkrescue`, and `xorriso`.

Build and link the kernel:

```sh
make ARCH=x86 KERNEL_BOOTLOADER=grub
```

That leaves `kamo.elf` in `out/x86/grub/`. To make the bootable ISO:

```sh
make ARCH=x86 KERNEL_BOOTLOADER=grub package
```

The ISO lands beside the ELF as `kamo.iso`. ISO is the only package format set
up right now; `make clean` clears the build output.

Using a differently named cross-toolchain? Set its prefix with
`CROSS_COMPILE`, for example `CROSS_COMPILE=i686-none-elf-`.


