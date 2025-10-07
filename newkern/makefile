# ------------------------------
# Paths
# ------------------------------
BOOTLOADER_SRC := bootloader/bootloader.s
BOOTLOADER_BIN := bootloader/bootloader.bin

ENTRY_SRC := entry/entry.s
ENTRY_BIN := entry/entry.bin

KERNEL_SRC := kernel/kernel.s
KERNEL_BIN := kernel/kernel.bin

BOOTABLE_BIN := os.img

NASM := nasm
NASMFLAGS := -f bin

# ------------------------------
# Dynamic NASM flags from config.mk
# ------------------------------
CONFIG_FILE := config.mk
CONFIG_VARS := $(shell sed -n 's/^\([A-Z_0-9]*\)[[:space:]]*=[[:space:]]*\(.*\)/\1=\2/p' $(CONFIG_FILE))
NASM_FLAGS := $(foreach v,$(CONFIG_VARS),-D$(v))

# ------------------------------
# Default target
# ------------------------------
all: bootable

# ------------------------------
# Build bootloader
# ------------------------------
$(BOOTLOADER_BIN): $(BOOTLOADER_SRC)
	@echo "Assembling bootloader..."
	$(NASM) $(NASMFLAGS) $(NASM_FLAGS) $< -o $@

# ------------------------------
# Build entry
# ------------------------------
$(ENTRY_BIN): $(ENTRY_SRC)
	@echo "Assembling entry..."
	KERNEL_SECTORS=$$(echo $$(( ($$(stat -c%s $(KERNEL_BIN)) + 511)/512 ))); \
	$(NASM) $(NASMFLAGS) -DKERNEL_SECTORS=$$KERNEL_SECTORS $(NASM_FLAGS) $< -o $@

# ------------------------------
# Build kernel
# ------------------------------
$(KERNEL_BIN): $(KERNEL_SRC)
	@echo "Assembling kernel..."
	# Compute sectors **after kernel is built** and pass to NASM
	$(NASM) $(NASMFLAGS) -DKERNEL_BUILD_DATE="$$(date)" -DKERNEL_SECTORS=$$KERNEL_SECTORS $(NASM_FLAGS) $< -o $@

# ------------------------------
# Bootable image
# ------------------------------
bootable: $(KERNEL_BIN) $(ENTRY_BIN) $(BOOTLOADER_BIN)
	@echo "Creating bootable image..."
	@dd if=/dev/zero of=$(BOOTABLE_BIN) bs=512 count=2880 status=none
	@dd if=$(BOOTLOADER_BIN) of=$(BOOTABLE_BIN) bs=512 count=1 conv=notrunc status=none
	@dd if=$(ENTRY_BIN) of=$(BOOTABLE_BIN) bs=512 seek=1 conv=notrunc status=none
	@dd if=$(KERNEL_BIN) of=$(BOOTABLE_BIN) bs=512 seek=2 conv=notrunc status=none
	@echo "Bootable image created: $(BOOTABLE_BIN)"

# ------------------------------
# Clean
# ------------------------------
clean:
	@echo "Cleaning..."
	@find . -type f -name "*.bin" -exec rm -f {} +
	@rm -f $(BOOTABLE_BIN)

# ------------------------------
# Phony targets
# ------------------------------
.PHONY: all bootable clean
