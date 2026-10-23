#include "psx/dma.h"

#include <assert.h>
#include <stdio.h>

static void prepare_one_word(psx_bus_t *bus, psx_memory_t *memory,
                             uint32_t dicr, uint32_t chcr)
{
    psx_bus_init(bus, memory);
    assert(psx_memory_write32(memory, 0x400u, 0xe1002222u));
    bus->dma_control = 1u << (2u * 4u + 3u);
    bus->dma_interrupt = dicr;
    bus->dma[2].madr = 0x400u;
    bus->dma[2].bcr = 1u;
    bus->dma[2].chcr = (1u << 24) | chcr;
}

int main(void)
{
    static psx_memory_t memory;
    psx_bus_t bus;

    psx_memory_init(&memory);

    /* Channel IRQ enable without master enable must not raise the aggregate IRQ. */
    prepare_one_word(&bus, &memory, 1u << 18, 1u);
    psx_dma_tick(&bus);
    assert((bus.dma_interrupt & (1u << 26)) != 0u);
    assert((bus.dma_interrupt & (1u << 31)) == 0u);
    assert((bus.irq_status & (1u << 3)) == 0u);

    /* Channel enable plus master enable raises DICR bit 31 and I_STAT bit 3. */
    prepare_one_word(&bus, &memory, (1u << 23) | (1u << 18), 1u);
    psx_dma_tick(&bus);
    assert((bus.dma_interrupt & (1u << 26)) != 0u);
    assert((bus.dma_interrupt & (1u << 31)) != 0u);
    assert((bus.irq_status & (1u << 3)) != 0u);

    /* Force IRQ is independent of per-channel enable. */
    prepare_one_word(&bus, &memory, 1u << 15, 1u);
    psx_dma_tick(&bus);
    assert((bus.dma_interrupt & (1u << 31)) != 0u);
    assert((bus.irq_status & (1u << 3)) != 0u);

    puts("PSXBlaster360 DMA IRQ tests: PASS");
    return 0;
}
