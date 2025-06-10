# Compiler and flags
# CC = gcc
CC = clang
CFLAGS = -g -O0 -Wall -I./include $(shell sdl2-config --cflags)
# CFLAGS = -g -O0 -fsanitize=address -fno-omit-frame-pointer -Wall -I./include $(shell sdl2-config --cflags)
LDFLAGS = $(shell sdl2-config --libs)

# Source and output
SRC_DIR = ./src
OBJ_DIR = ./obj
BIN = emulator

# Files
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# Default target
all: $(BIN)

# Link
$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compile
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Ensure obj directory exists
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Clean up
clean:
	rm -rf $(OBJ_DIR) $(BIN)

.PHONY: all clean