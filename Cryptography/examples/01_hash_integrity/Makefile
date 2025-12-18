# Example 01: Hash and Integrity
# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O2 -I/usr/include/openssl
LDFLAGS = -lssl -lcrypto

# Directories
SRC_DIR = src
BIN_DIR = bin

# Source files
SOURCES = $(wildcard $(SRC_DIR)/*.c)
TARGETS = $(patsubst $(SRC_DIR)/%.c,$(BIN_DIR)/%,$(SOURCES))

# Default target
all: $(BIN_DIR) $(TARGETS)

# Create bin directory
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Compile each source file
$(BIN_DIR)/%: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# Clean
clean:
	rm -rf $(BIN_DIR)

# Phony targets
.PHONY: all clean
