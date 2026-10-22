#ifndef PSX_DMA_H
#define PSX_DMA_H

#include "psx/bus.h"

/* Advance the currently modeled DMA channels by one bus tick. */
void psx_dma_tick(psx_bus_t *bus);

#endif
