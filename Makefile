CC ?= cc
CPPFLAGS ?= -Iinclude
CFLAGS ?= -std=c99 -O2 -Wall -Wextra -Werror -pedantic
BUILD_DIR ?= build

.PHONY: all host-test clean

all: host-test

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/test_memory: src/core/memory.c tests/test_memory.c include/psx/memory.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) src/core/memory.c tests/test_memory.c -o $@

host-test: $(BUILD_DIR)/test_memory
	./$(BUILD_DIR)/test_memory

clean:
	rm -rf $(BUILD_DIR)
