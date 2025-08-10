# Compiler and flags
CC = clang
CFLAGS = -g3 -O0 -Wall -I./include $(shell sdl2-config --cflags)
LDFLAGS = $(shell sdl2-config --libs)

# Directories
SRC_DIR = ./src
OBJ_DIR = ./obj
TEST_DIR = ./test
TEST_OBJ_DIR = $(OBJ_DIR)/test

# Output binaries
BIN = emulator
TEST_BIN = test_runner

# Source files
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# Exclude main.c for test build
SHARED_SRCS = $(filter-out $(SRC_DIR)/main.c, $(SRCS))
SHARED_OBJS = $(patsubst $(SRC_DIR)/%.c, $(TEST_OBJ_DIR)/%.o, $(SHARED_SRCS))

# Test files
TEST_SRCS = $(wildcard $(TEST_DIR)/*.c)
TEST_OBJS = $(patsubst $(TEST_DIR)/%.c, $(TEST_OBJ_DIR)/%.o, $(TEST_SRCS))

# Default target
all: $(BIN)

# Main binary
$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Test binary
test: $(TEST_BIN)

$(TEST_BIN): $(TEST_OBJS) $(SHARED_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compile source to obj
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile shared src for tests (excluding main.c)
$(TEST_OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(TEST_OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile test code
$(TEST_OBJ_DIR)/%.o: $(TEST_DIR)/%.c | $(TEST_OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Ensure directories
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(TEST_OBJ_DIR):
	mkdir -p $(TEST_OBJ_DIR)

# Clean
clean:
	rm -rf $(OBJ_DIR) $(TEST_OBJ_DIR) $(BIN) $(TEST_BIN)

# Run a test with a specific YAML file
run_test: test
	@if [ -z "$(FILE)" ]; then \
		echo "Usage: make run_test FILE=path/to/file.yaml"; \
		exit 0; \
	fi; \
	./$(TEST_BIN) "$(FILE)"

# Run tests for all .yaml files in the test directory
run_tests: test
	@for f in $(wildcard $(TEST_DIR)/*.yaml); do \
		echo "Running test on $$f"; \
		./$(TEST_BIN) "$$f" || { echo "Test failed on $$f"; exit 1; }; \
	done

.PHONY: all clean test