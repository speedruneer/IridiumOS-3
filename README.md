# How to build?

## building kernelgr

Not in the makefile, you will need GRC though, it is not properly functional (latest working version is not on git).

## building main kernel

run `make defaultconfig && make menuconfig` to configure the kernel and run `make kernel` to build the kernel.

## building bootloader

run `make bootloader`.

## building bootable image

run `make bootable`.