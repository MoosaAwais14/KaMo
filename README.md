# KaMo

KaMo is an operating system built from scratch, with the goal of creating a fully functional OS to explore low-level systems programming.

This repository contains the source code and development files for KaMo, including:

- **Kernel:** The core of the operating system
- **Userspace:** Software and components that run outside the kernel.
- **Build System:** Configuration and files used to compile the project and build the OS image
- **OS Image:** Files and build logic for assembling the operating system into an image

## Building

Build the project using Make.
Make is a build automation tool that reads instructions from a Makefile to compile code and build a project. For KaMo, running make executes the configured build steps needed to compile the OS.

## Repository Structure

- **kernel/**: Kernel source code.
- **userspace**: Userspace source code.
- **image**: OS image build files.
- **kamo/**: Main project components
- **Makefile:** Defines the build process for the project
- **config.mk**: Defines the compiler toolchain and build configuration
- **Dockerfile**: Defines the docker build environment
