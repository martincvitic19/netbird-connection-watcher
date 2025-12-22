# Cross compiler
CC := mipsel-linux-musl-gcc

# Directories
BUILD_DIR := build

# Output binary name
TARGET := netbird_connection_watcher
OUT := $(BUILD_DIR)/$(TARGET)

# Source files
SRC := main.c

# Compiler flags
CFLAGS := -Os -march=mips32r2 -static
LDFLAGS :=

# Default target
all: $(OUT)

# Create build dir if it doesn't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Build rule
$(OUT): $(SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SRC) -o $(OUT) $(LDFLAGS)
	chmod +x $(OUT)

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)
