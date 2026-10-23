#include "psx/dma.h"

#include <assert.h>
#include <stdio.h>

int main(void)
{
    static psx_memory_t memory;
    psx_bus_t bus;
    uint32_t value = 0u;

    psx_memory_init(&memory);
    psx_bus_init(&bus, &memory);

    /* Enable the DMA2 completion interrupt and verify DPCR gating first. */
    assert(psx_bus_write32(&bus, 0x1f8010f4u, (1u << 23) | (1u << 18)));

    /* A started channel stays pending while DPCR disables channel 2. */
    assert(psx_memory_write32(&memory, 0x200u, 0xe1001111u));
    bus.dma[2].madr = 0x200u;
    bus.dma[2].bcr = 1u;
    bus.dma[2].chcr = (1u << 24) | 1u;
    psx_dma_tick(&bus);
    assert((bus.dma[2].chcr & (1u << 24)) != 0u);
    assert(bus.dma_words_transferred == 0u);

    /* Enable channel 2 and verify a single RAM -> GP0 word transfer. */
    bus.dma_control = 1u << (2u * 4u + 3u);
    psx_dma_tick(&bus);
    assert(bus.dma_words_transferred == 1u);
    assert(bus.gpu_draw_mode == 0x1111u);
    assert(bus.dma[2].madr == 0x204u);
    assert((bus.dma[2].chcr & (1u << 24)) == 0u);
    assert((bus.dma_interrupt & (1u << 26)) != 0u);
    assert((bus.dma_interrupt & (1u << 31)) != 0u);
    assert((bus.irq_status & (1u << 3)) != 0u);

    /* DICR completion flags are W1C; clearing the last enabled flag drops master IRQ. */
    assert(psx_bus_write32(&bus, 0x1f8010f4u,
                           (1u << 23) | (1u << 18) | (1u << 26)));
    assert((bus.dma_interrupt & (1u << 26)) == 0u);
    assert((bus.dma_interrupt & (1u << 31)) == 0u);
    /* I_STAT is separately acknowledged by the CPU. */
    assert((bus.irq_status & (1u << 3)) != 0u);
    assert(psx_bus_write32(&bus, 0x1f801070u, 1u << 3));
    assert((bus.irq_status & (1u << 3)) == 0u);

    /* Decrement mode updates MADR in the reverse direction. */
    assert(psx_memory_write32(&memory, 0x300u, 0xe6000002u));
    bus.dma[2].madr = 0x300u;
    bus.dma[2].bcr = 1u;
    bus.dma[2].chcr = (1u << 24) | (1u << 1) | 1u;
    psx_dma_tick(&bus);
    assert(bus.gpu_mask_setting == 2u);
    assert(bus.dma[2].madr == 0x2fcu);
    assert(bus.dma_words_transferred == 2u);

    /* Unsupported sync mode ends safely without transferring data. */
    bus.dma[2].madr = 0x200u;
    bus.dma[2].bcr = 1u;
    bus.dma[2].chcr = (1u << 24) | (1u << 9) | 1u;
    psx_dma_tick(&bus);
    assert((bus.dma[2].chcr & (1u << 24)) == 0u);
    assert(bus.dma_words_transferred == 2u);
    assert(psx_memory_read32(&memory, 0x200u, &value) && value == 0xe1001111u);

    puts("PSXBlaster360 DMA tests: PASS");
    return 0;
}
