# Makefile for SONAR Project
# Author: SONAR Development Team
# Version: 1.0.0

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 -Wno-cast-function-type
DEBUG_FLAGS = -g -DDEBUG
PIC_FLAGS = -fPIC
LDFLAGS = -lm
SHARED_LDFLAGS = -shared

# Directories
SRC_DIR = src
CORE_DIR = $(SRC_DIR)/core
MAIN_DIR = $(SRC_DIR)/main
MODULES_DIR = $(SRC_DIR)/modules/default
INCLUDE_DIR = include
BUILD_DIR = build
BIN_DIR = $(BUILD_DIR)/bin
OBJ_DIR = $(BUILD_DIR)/obj
LIB_DIR = lib
DOCS_DIR = docs/Documentation

# Source files
MAIN_SRC = $(MAIN_DIR)/mojibake_sonar.c
MOJIBAKE_SRC = $(CORE_DIR)/mojibake.c
MODULE_SRCS = $(MODULES_DIR)/mbx_default.c \
              $(MODULES_DIR)/mbx_charcount.c \
              $(MODULES_DIR)/mbx_textview.c \
              $(MODULES_DIR)/mbx_sonar.c \
              $(MODULES_DIR)/mbx_dsonar.c

# Object files
MAIN_OBJ = $(OBJ_DIR)/mojibake_sonar.o
MOJIBAKE_OBJ = $(OBJ_DIR)/mojibake.o
MODULE_OBJS = $(OBJ_DIR)/mbx_default.o \
              $(OBJ_DIR)/mbx_charcount.o \
              $(OBJ_DIR)/mbx_textview.o \
              $(OBJ_DIR)/mbx_sonar.o \
              $(OBJ_DIR)/mbx_dsonar.o
ALL_OBJS = $(MAIN_OBJ) $(MOJIBAKE_OBJ) $(MODULE_OBJS)

# Target executable
TARGET = mojibake_sonar
DEBUG_TARGET = mojibake_sonar_debug
SHARED_LIB = libmojibake_sonar.so

# Shared library object files (compiled with -fPIC)
SHARED_OBJS = $(OBJ_DIR)/mojibake_shared.o \
              $(OBJ_DIR)/mbx_default_shared.o \
              $(OBJ_DIR)/mbx_charcount_shared.o \
              $(OBJ_DIR)/mbx_textview_shared.o \
              $(OBJ_DIR)/mbx_sonar_shared.o \
              $(OBJ_DIR)/mbx_dsonar_shared.o

# Include paths
INCLUDES = -I$(INCLUDE_DIR) -I$(CORE_DIR) -I$(MODULES_DIR)

# Default target
all: $(TARGET)

# Create directories if they don't exist
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Main target
$(TARGET): $(ALL_OBJS) | $(BIN_DIR)
	$(CC) $(ALL_OBJS) -o $(BIN_DIR)/$(TARGET) $(LDFLAGS)
	@echo "Build complete: $(BIN_DIR)/$(TARGET)"

# Debug target
debug: CFLAGS += $(DEBUG_FLAGS)
debug: $(DEBUG_TARGET)

$(DEBUG_TARGET): $(ALL_OBJS) | $(BIN_DIR)
	$(CC) $(ALL_OBJS) -o $(BIN_DIR)/$(DEBUG_TARGET) $(LDFLAGS)
	@echo "Debug build complete: $(BIN_DIR)/$(DEBUG_TARGET)"

# Shared library target
shared: $(SHARED_LIB)

$(SHARED_LIB): $(SHARED_OBJS) | $(BIN_DIR)
	$(CC) $(SHARED_LDFLAGS) $(SHARED_OBJS) -o $(BIN_DIR)/$(SHARED_LIB) $(LDFLAGS)
	@echo "Shared library build complete: $(BIN_DIR)/$(SHARED_LIB)"

# Compile main source
$(MAIN_OBJ): $(MAIN_SRC) | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Compile mojibake core
$(MOJIBAKE_OBJ): $(MOJIBAKE_SRC) | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Compile module sources
$(OBJ_DIR)/%.o: $(MODULES_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Compile shared library objects with PIC
$(OBJ_DIR)/mojibake_shared.o: $(MOJIBAKE_SRC) | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(PIC_FLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/%_shared.o: $(MODULES_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(PIC_FLAGS) $(INCLUDES) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(ALL_OBJS) $(SHARED_OBJS)
	rm -f $(BIN_DIR)/$(TARGET)
	rm -f $(BIN_DIR)/$(DEBUG_TARGET)
	rm -f $(BIN_DIR)/$(SHARED_LIB)
	@echo "Clean complete"

# Clean everything including build directory
clean-all: clean
	rm -rf $(BUILD_DIR)
	@echo "Full clean complete"

# Install target (optional)
install: $(TARGET)
	cp $(BIN_DIR)/$(TARGET) /usr/local/bin/
	@echo "Installed to /usr/local/bin/"

# Test targets
test-hex: $(TARGET)
	./$(BIN_DIR)/$(TARGET) test_file.bin hex

test-sonar: $(TARGET)
	./$(BIN_DIR)/$(TARGET) test_file.bin sonar

test-dsonar: $(TARGET)
	./$(BIN_DIR)/$(TARGET) test_audio.wav dsonar

# Help target
help:
	@echo "Mojibake SONAR Build System"
	@echo "Available targets:"
	@echo "  all        - Build release version (default)"
	@echo "  debug      - Build debug version with symbols"
	@echo "  shared     - Build shared library (.so file)"
	@echo "  clean      - Remove object files and executables"
	@echo "  clean-all  - Remove all build artifacts including bin/"
	@echo "  install    - Install to /usr/local/bin/"
	@echo "  test-hex   - Test with hex module"
	@echo "  test-sonar - Test with SONAR module"
	@echo "  test-dsonar- Test with dSONAR module"
	@echo "  help       - Show this help message"
	@echo ""
	@echo "Usage examples:"
	@echo "  make                    # Build release version"
	@echo "  make debug              # Build debug version"
	@echo "  make CC=clang           # Use clang compiler"
	@echo "  make CFLAGS='-O3 -Wall' # Custom compiler flags"

# Phony targets
.PHONY: all debug shared clean clean-all install test-hex test-sonar test-dsonar help

# Dependencies (basic)
$(MAIN_OBJ): $(INCLUDE_DIR)/mojibake/mojibake.h $(MODULES_DIR)/mbx_default.h $(MODULES_DIR)/mbx_sonar.h $(MODULES_DIR)/mbx_dsonar.h
$(MOJIBAKE_OBJ): $(INCLUDE_DIR)/mojibake/mojibake.h
$(OBJ_DIR)/mbx_sonar.o: $(MODULES_DIR)/mbx_sonar.h $(INCLUDE_DIR)/mojibake/mojibake.h
$(OBJ_DIR)/mbx_dsonar.o: $(MODULES_DIR)/mbx_dsonar.h $(INCLUDE_DIR)/mojibake/mojibake.h
$(OBJ_DIR)/mbx_default.o: $(MODULES_DIR)/mbx_default.h $(INCLUDE_DIR)/mojibake/mojibake.h
$(OBJ_DIR)/mbx_charcount.o: $(MODULES_DIR)/mbx_charcount.h $(INCLUDE_DIR)/mojibake/mojibake.h
$(OBJ_DIR)/mbx_textview.o: $(MODULES_DIR)/mbx_textview.h $(INCLUDE_DIR)/mojibake/mojibake.h