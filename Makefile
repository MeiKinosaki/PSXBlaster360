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

$(BUILD_DIR)/test_system: src/core/system.c src/core/bus.c src/core/timers.c src/core/dma.c src/core/cpu.c src/core/memory.c tests/test_system.c include/psx/system.h include/psx/bus.h include/psx/cpu.h include/psx/memory.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) src/core/system.c src/core/bus.c src/core/timers.c src/core/dma.c src/core/cpu.c src/core/memory.c tests/test_system.c -o $@

$(BUILD_DIR)/test_dma: src/core/dma.c src/core/bus.c src/core/timers.c src/core/memory.c tests/test_dma.c include/psx/dma.h include/psx/bus.h include/psx/memory.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) src/core/dma.c src/core/bus.c src/core/timers.c src/core/memory.c tests/test_dma.c -o $@

$(BUILD_DIR)/test_bus: src/core/bus.c src/core/timers.c src/core/dma.c src/core/memory.c tests/test_bus.c include/psx/bus.h include/psx/memory.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) src/core/bus.c src/core/timers.c src/core/dma.c src/core/memory.c tests/test_bus.c -o $@

$(BUILD_DIR)/test_cpu: src/core/cpu.c src/core/bus.c src/core/timers.c src/core/dma.c src/core/memory.c tests/test_cpu.c include/psx/cpu.h include/psx/memory.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) src/core/cpu.c src/core/bus.c src/core/timers.c src/core/dma.c src/core/memory.c tests/test_cpu.c -o $@

host-test: $(BUILD_DIR)/test_memory $(BUILD_DIR)/test_cpu $(BUILD_DIR)/test_bus $(BUILD_DIR)/test_dma $(BUILD_DIR)/test_system
	./$(BUILD_DIR)/test_memory
	./$(BUILD_DIR)/test_cpu
	./$(BUILD_DIR)/test_bus
	./$(BUILD_DIR)/test_dma
	./$(BUILD_DIR)/test_system

clean:
	rm -rf $(BUILD_DIR)
