# Default config file
CONFIG_FILE := config.mk
DEFAULT_CONFIG := default.mk

# Include config file if it exists
-include $(CONFIG_FILE)

# ------------------------------
# Variables
# ------------------------------
BOOTLOADER_PATH ?= bootloader
BOOTLOADER_SRC  := $(BOOTLOADER_PATH)/bootloader.s
BOOTLOADER_BIN  := $(BOOTLOADER_PATH)/bootloader.bin

KERNEL_BIN      := $(KERNEL_PATH)kernel.bin  # kernel output path

# ------------------------------
# Dynamic NASM flags
# ------------------------------
BOOTLOADER_FLAGS := -DKERNEL_SECTORS=$(KERNEL_SECTORS)
ifeq ($(VGA), true)
BOOTLOADER_FLAGS += -DVGA
endif
ifeq ($(DISK_AHCI), true)
BOOTLOADER_FLAGS += -DAHCI
endif

KERNEL_FLAGS := -DKERNEL_NAME=$(KERNEL_NAME)
ifeq ($(VGA), true)
KERNEL_FLAGS += -DVGA
endif
ifeq ($(DISK_AHCI), true)
KERNEL_FLAGS += -DAHCI
endif

# ------------------------------
# Default target
# ------------------------------
all: build

# ------------------------------
# menuconfig
# ------------------------------
menuconfig:
	@echo "Launching menuconfig..."
	@luajit conf.lua

# ------------------------------
# Reset config
# ------------------------------
defaultconfig:
	@echo "Resetting config..."
	@rm -f $(CONFIG_FILE)
	@cp $(DEFAULT_CONFIG) $(CONFIG_FILE)

# ------------------------------
# Bootloader compilation
# ------------------------------
bootloader: $(BOOTLOADER_BIN)

$(BOOTLOADER_BIN): $(BOOTLOADER_SRC)
	@echo "Assembling bootloader..."
	nasm -f bin $(BOOTLOADER_SRC) -o $(BOOTLOADER_BIN) $(BOOTLOADER_FLAGS)

# ------------------------------
# Kernel compilation
# ------------------------------
# ------------------------------
# Kernel compilation
# ------------------------------
kernel:
	@echo "Assembling Kernel..."
	nasm -f bin $(KERNEL_FLAGS) $(KERNEL_PATH)kernel.s -o $(KERNEL_BIN)

# ------------------------------
# Build combined bootable image
# ------------------------------
bootable: bootloader kernel
	@echo "Creating bootable image..."
	@cp $(BOOTLOADER_BIN) bootable.bin
	@dd if=$(KERNEL_BIN) of=bootable.bin bs=512 seek=1 conv=notrunc status=none
	@echo "Bootable image created: bootable.bin"

# ------------------------------
# Build target placeholder
# ------------------------------
build: bootable
	@echo "Build complete."

# ------------------------------
# Clean
# ------------------------------
clean:
	@echo "Cleaning..."
	@find . -type f -name "*.bin" -exec rm -f {} +

# ------------------------------
# Phony targets
# ------------------------------
.PHONY: all build menuconfig defaultconfig bootloader kernel bootable clean
