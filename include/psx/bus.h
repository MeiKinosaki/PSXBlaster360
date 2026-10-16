#ifndef PSX_BUS_H
#define PSX_BUS_H

#include <stdbool.h>
#include <stdint.h>
#include "psx/memory.h"

#define PSX_BUS_TIMER_COUNT 3u
#define PSX_BUS_DMA_CHANNELS 7u

typedef struct psx_timer {
    uint32_t count;
    uint32_t mode;
    uint32_t target;
} psx_timer_t;

typedef struct psx_dma_channel {
    uint32_t madr;
    uint32_t bcr;
    uint32_t chcr;
} psx_dma_channel_t;

typedef struct psx_bus {
    psx_memory_t *memory;
    uint32_t irq_status;
    uint32_t irq_mask;
    psx_timer_t timers[PSX_BUS_TIMER_COUNT];
    psx_dma_channel_t dma[PSX_BUS_DMA_CHANNELS];
    uint32_t dma_control;
    uint32_t dma_interrupt;
    uint32_t gpu_draw_mode;
    uint32_t gpu_texture_window;
    uint32_t gpu_draw_top_left;
    uint32_t gpu_draw_bottom_right;
    uint32_t gpu_draw_offset;
    uint32_t gpu_mask_setting;
    uint32_t gpu_gp1_status;
    uint64_t gpu_gp0_write_count;
    uint64_t gpu_gp1_write_count;
} psx_bus_t;

void psx_bus_init(psx_bus_t *bus, psx_memory_t *memory);
bool psx_bus_read32(psx_bus_t *bus, uint32_t address, uint32_t *value);
bool psx_bus_write32(psx_bus_t *bus, uint32_t address, uint32_t value);
void psx_bus_tick(psx_bus_t *bus, uint32_t cycles);
void psx_bus_gpu_gp0_write(psx_bus_t *bus, uint32_t value);

#endif
