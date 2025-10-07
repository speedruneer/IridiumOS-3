# ------------------------------
# Config
# ------------------------------
CONFIG_FILE ?= config.mk
DEFAULT_CONFIG := default.mk

-include $(CONFIG_FILE)

# ------------------------------
# Paths
# ------------------------------
BOOTLOADER_PATH ?= bootloader
BOOTLOADER_SRC  := $(BOOTLOADER_PATH)/bootloader.s
BOOTLOADER_BIN  := $(BOOTLOADER_PATH)/bootloader.bin

ENTRY_PATH ?= entry
ENTRY_SRC  := $(ENTRY_PATH)/entry.s
ENTRY_BIN  := $(ENTRY_PATH)/entry.bin

KERNEL_PATH ?= kernel
KERNEL_SRC  := $(KERNEL_PATH)/kernel.s
KERNEL_BIN  := $(KERNEL_PATH)/kernel.bin

BOOTABLE_BIN := os.img

# ------------------------------
# Compute kernel sectors dynamically
# ------------------------------
KERNEL_SECTORS := $(shell stat -c%s $(KERNEL_BIN) 2>/dev/null | awk '{printf "%d\n", ($$1 + 511)/512}')

# ------------------------------
# NASM flags: automatic from config
# ------------------------------
# Every variable in config.mk becomes a -D flag automatically
CONFIG_VARS := $(shell sed -n 's/^\([A-Z_0-9]*\) *=.*/\1/p' $(CONFIG_FILE) 2>/dev/null)
NASM_FLAGS := $(foreach v,$(CONFIG_VARS),-D$(v)=$($(v)))
NASM_FLAGS += -DKERNEL_SECTORS=$(KERNEL_SECTORS)

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
# Bootloader
# ------------------------------
bootloader: $(BOOTLOADER_BIN)

$(BOOTLOADER_BIN): $(BOOTLOADER_SRC)
	@echo "Assembling bootloader..."
	@$(NASM) -f bin $(NASM_FLAGS) $< -o $@

# ------------------------------
# Entry
# ------------------------------
entry: $(ENTRY_BIN)

$(ENTRY_BIN): $(ENTRY_SRC)
	@echo "Assembling entry..."
	@$(NASM) -f bin $(NASM_FLAGS) $< -o $@

# ------------------------------
# Kernel
# ------------------------------
kernel: $(KERNEL_BIN)

$(KERNEL_BIN): $(KERNEL_SRC)
	@echo "Assembling kernel..."
	$(NASM) -f bin -DKERNEL_BUILD_DATE="$(shell date)" $(NASM_FLAGS) $< -o $@

# ------------------------------
# Bootable image
# ------------------------------
bootable: bootloader entry kernel
	@echo "Creating bootable image..."
	@dd if=/dev/zero of=$(BOOTABLE_BIN) bs=512 count=2880 status=none
	@dd if=$(BOOTLOADER_BIN) of=$(BOOTABLE_BIN) bs=512 count=1 conv=notrunc status=none
	@dd if=$(ENTRY_BIN) of=$(BOOTABLE_BIN) bs=512 seek=1 conv=notrunc status=none
	@dd if=$(KERNEL_BIN) of=$(BOOTABLE_BIN) bs=512 seek=2 conv=notrunc status=none
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
.PHONY: all build menuconfig defaultconfig bootloader entry kernel bootable clean
