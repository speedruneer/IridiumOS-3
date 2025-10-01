# Makefile

# Default config file
CONFIG_FILE := config.mk

# Include config file if it exists
-include $(CONFIG_FILE)

# Default target
all: default

# Default target: just print current config
default: 
	@echo "Building with config:"
	@echo "ENABLE_USB=$(ENABLE_USB)"
	@echo "USE_EXT4=$(USE_EXT4)"
	@echo "CFLAGS=$(CFLAGS)"

# menuconfig target: run your Lua script
menuconfig:
	@echo "Launching menuconfig..."
	@luajit conf.lua

# defaultconfig: create a default config if none exists
defaultconfig:
	@echo "Creating default config..."
	@touch $(CONFIG_FILE)
	@echo "ENABLE_USB=1" > $(CONFIG_FILE)
	@echo "USE_EXT4=0" >> $(CONFIG_FILE)

# Clean: remove generated files
clean:
	@echo "Cleaning..."
	@rm -f $(CONFIG_FILE)

.PHONY: all default menuconfig defaultconfig clean
