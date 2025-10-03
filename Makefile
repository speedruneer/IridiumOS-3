# ------------------------------
# Default config file
# ------------------------------
CONFIG_FILE ?= config.mk
DEFAULT_CONFIG := default.mk

# Include config file if it exists
-include $(CONFIG_FILE)

# ------------------------------
# Paths
# ------------------------------
BOOTLOADER_PATH ?= bootloader
BOOTLOADER_SRC  := $(BOOTLOADER_PATH)/bootloader.s
BOOTLOADER_BIN  := $(BOOTLOADER_PATH)/bootloader.bin

KERNEL_BIN      := $(KERNEL_PATH)/kernel.bin
KERNEL_SRC      := $(KERNEL_PATH)/kernel.s

BOOTABLE_BIN    := bootable.bin

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

KERNEL_FLAGS := -DKERNEL_NAME=$(KERNEL_NAME) -DKERNEL_SECTORS=$(KERNEL_SECTORS)
ifeq ($(VGA), true)
KERNEL_FLAGS += -DVGA
endif
ifeq ($(DISK_AHCI), true)
KERNEL_FLAGS += -DAHCI
endif

# Add future config keys automatically
ifdef DISPLAY_USE_PRINT
KERNEL_FLAGS += -DDISPLAY_USE_PRINT
endif
ifdef FS_USE_RWFS
KERNEL_FLAGS += -DFS_USE_RWFS
endif
ifdef FS_USE_FAT12
KERNEL_FLAGS += -DFS_USE_FAT12
endif
ifdef FS_USE_FAT16
KERNEL_FLAGS += -DFS_USE_FAT16
endif
ifdef FS_USE_FAT32
KERNEL_FLAGS += -DFS_USE_FAT32
endif
ifdef FS_USE_TMPFS
KERNEL_FLAGS += -DFS_USE_TMPFS
endif

# ------------------------------
# Default target
# ------------------------------
all: build

# ------------------------------
# Menuconfig
# ------------------------------
menuconfig:
	@echo "Launching menuconfig..."
	@luajit conf.lua

# ------------------------------
# Reset config
# ------------------------------
defaultconfig:
	@echo "Resetting config..."
	@cp -f $(DEFAULT_CONFIG) $(CONFIG_FILE)

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
kernel: $(KERNEL_BIN)

$(KERNEL_BIN): $(KERNEL_SRC)
	@echo "Assembling kernel..."
	@echo $(KERNEL_FLAGS)
	nasm -f bin -DKERNEL_BUILD_DATE="$(shell date)" $(KERNEL_FLAGS) $(KERNEL_SRC) -o $(KERNEL_BIN)

# ------------------------------
# Bootable image
# ------------------------------
bootable: bootloader kernel
	@echo "Creating bootable image..."
	@cp -f $(BOOTLOADER_BIN) $(BOOTABLE_BIN)
	@dd if=$(KERNEL_BIN) of=$(BOOTABLE_BIN) bs=512 seek=1 conv=notrunc status=none
	@echo "Bootable image created: $(BOOTABLE_BIN)"

# ------------------------------
# Build target
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
