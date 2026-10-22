#include "psx/dma.h"

#include <stddef.h>

#define DMA_IRQ_BIT(channel) (1u << (24u + (channel)))
#define DMA_IRQ_ENABLE(channel) (1u << (16u + (channel)))

static void dma_finish(psx_bus_t *bus, uint32_t channel)
{
    bus->dma[channel].chcr &= ~(1u << 24);
    bus->dma_interrupt |= DMA_IRQ_BIT(channel);
    if ((bus->dma_interrupt & (1u << 15)) != 0u ||
        ((bus->dma_interrupt & DMA_IRQ_ENABLE(channel)) != 0u &&
         (bus->dma_interrupt & (1u << 23)) != 0u)) {
        bus->dma_interrupt |= 1u << 31;
    }
    if ((bus->dma_interrupt & (1u << 31)) != 0u)
        bus->irq_status |= 1u << 3;
}

/* Manual-mode RAM-to-GPU DMA. Unsupported modes are stopped safely. */
void psx_dma_tick(psx_bus_t *bus)
{
    psx_dma_channel_t *dma;
    uint32_t words, i, address, value;
    if (bus == NULL || bus->memory == NULL) return;
    dma = &bus->dma[2];
    if ((dma->chcr & (1u << 24)) == 0u) return;
    if ((bus->dma_control & (1u << (2u * 4u + 3u))) == 0u) return;
    if ((dma->chcr & 1u) == 0u || ((dma->chcr >> 9) & 3u) != 0u) {
        dma_finish(bus, 2u);
        return;
    }
    words = dma->bcr & 0xffffu;
    if (words == 0u) words = 65536u;
    address = dma->madr & 0x00fffffcu;
    for (i = 0; i < words; ++i) {
        if (!psx_memory_read32(bus->memory, address, &value)) {
            dma_finish(bus, 2u);
            return;
        }
        psx_bus_gpu_gp0_write(bus, value);
        bus->dma_words_transferred++;
        address = (dma->chcr & (1u << 1)) != 0u ? address - 4u : address + 4u;
        address &= 0x00ffffffu;
    }
    dma->madr = address;
    dma->bcr &= 0xffff0000u;
    dma_finish(bus, 2u);
}
