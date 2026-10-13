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

$(BUILD_DIR)/test_cpu: src/core/cpu.c src/core/memory.c tests/test_cpu.c include/psx/cpu.h include/psx/memory.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) src/core/cpu.c src/core/memory.c tests/test_cpu.c -o $@

host-test: $(BUILD_DIR)/test_memory $(BUILD_DIR)/test_cpu
	./$(BUILD_DIR)/test_memory
	./$(BUILD_DIR)/test_cpu

clean:
	rm -rf $(BUILD_DIR)
