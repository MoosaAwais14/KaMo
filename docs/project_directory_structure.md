# Project Directory Overview	
The KaMo project is, in essence, an operating system project as a whole. For this reason, and for other organizational purposes, the project is split into three main parts: **bootloader compliance, the kernel, and userspace (system) programs**.

## kamo/
This folder contains the bootloader compliance code.
### Why?
The idea behind this separation is to more clearly define the responsibilities of KaMo's kernel. Rather than having the kernel itself be responsible for everything required to satisfy the bootloader's expectations, those responsibilities are isolated within *kamo/*.

This creates a clearer boundary between what is required to boot KaMo and what is required to run KaMo. The bootloader compliance layer handles the former, while the kernel can remain focused on its own responsibilities as an operating system kernel.
### Structure
Within the *kamo/* folder are multiple implementations for different architectures. For example, *kamo/x86/* contains the implementation specific to the x86 architecture.

Within each architecture, there are further directories for the bootloaders or boot protocols that KaMo supports. For example, *kamo/x86/GRUB2/* contains the implementation for booting KaMo through GRUB 2 on x86.
### Responsibilities
The expectation for each bootloader-compliant entry implementation is to ready the machine into a "runnable" state, as defined by KaMo's standards.

--- This is the end of *kamo/* responsibilities for this document ---

## kernel/
This folder contains the kernel code.
### Structure
There are two main parts of *kernel/*: generic kernel and architectrual specific kernel. Not in order, *kernel/arch/* contains the architectrual specific kernel, *kernel/* (literally anything under the folder that isnt under *kernel/arch/*) contains the generic kernel.
### Why utilize this seperation?
The KaMo project is designed to support multiple machine or computer architectures. Whether such "expectations" will be reached or not, this separation provides better scalability and modularity, further enforcing a clearer separation between "low-level hardware code" and higher conceptual ideas. This prevents hardware-specific code and higher-level concepts from mixing together and clouding the project's vision.

## userspace/
This folder contains the userspace programs and system programs for KaMo.

--- This section is not completed: Project development has not reached this stage ---
