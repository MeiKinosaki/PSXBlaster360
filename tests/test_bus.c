#include "psx/bus.h"

#include <assert.h>
#include <stdio.h>

int main(void)
{
    static psx_memory_t memory;
    psx_bus_t bus;
    uint32_t value = 0;

    psx_memory_init(&memory);
    psx_bus_init(&bus, &memory);
    assert(psx_bus_write32(&bus, 0x100u, 0x12345678u));
    assert(psx_bus_read32(&bus, 0x100u, &value) && value == 0x12345678u);

    assert(psx_bus_write32(&bus, 0x1f801074u, 0x7ffu));
    assert(psx_bus_read32(&bus, 0x1f801074u, &value) && value == 0x7ffu);
    bus.irq_status = 0x15u;
    assert(psx_bus_write32(&bus, 0x1f801070u, 0x11u)); /* acknowledge IRQ bits 0 and 4 */
    assert(bus.irq_status == 0x4u);

    assert(psx_bus_write32(&bus, 0x1f801108u, 10u)); /* timer 0 target */
    assert(psx_bus_write32(&bus, 0x1f801104u, 1u << 4)); /* IRQ at target */
    psx_bus_tick(&bus, 12u);
    assert((bus.timers[0].mode & (1u << 11)) != 0u);
    assert((bus.irq_status & (1u << 4)) != 0u);

    assert(psx_bus_write32(&bus, 0x1f801080u, 0x00123456u)); /* DMA0 MADR */
    assert(psx_bus_write32(&bus, 0x1f801084u, 0x00010002u));
    assert(psx_bus_read32(&bus, 0x1f801080u, &value) && value == 0x00123456u);
    assert(psx_bus_write32(&bus, 0x1f8010f0u, 0x07654b21u));
    assert(psx_bus_read32(&bus, 0x1f8010f0u, &value) && value == 0x07654b21u);

    /* Large cycle deltas must wrap the 16-bit timer without 32-bit overflow. */
    bus.timers[1].count = 0xfff0u;
    bus.timers[1].target = 0u;
    psx_bus_tick(&bus, 0x10020u);
    assert(bus.timers[1].count == 0x0010u);
    assert((bus.timers[1].mode & (1u << 12)) != 0u);

    /* Manual RAM-to-GPU DMA channel 2 transfers two GP0 words. */
    assert(psx_memory_write32(&memory, 0x200u, 0xe1001111u));
    assert(psx_memory_write32(&memory, 0x204u, 0xe6000001u));
    assert(psx_bus_write32(&bus, 0x1f8010f4u, (1u << 23) | (1u << 18)));
    assert(psx_bus_write32(&bus, 0x1f8010a0u, 0x200u)); /* DMA2 MADR */
    assert(psx_bus_write32(&bus, 0x1f8010a4u, 2u)); /* two words */
    assert(psx_bus_write32(&bus, 0x1f8010a8u, (1u << 24) | 1u)); /* start, RAM->GPU */
    psx_bus_tick(&bus, 1u);
    assert(bus.dma_words_transferred == 2u);
    assert(bus.gpu_draw_mode == 0x1111u);
    assert(bus.gpu_mask_setting == 1u);
    assert(bus.dma[2].madr == 0x208u);
    assert((bus.dma_interrupt & (1u << 26)) != 0u);
    assert((bus.irq_status & (1u << 3)) != 0u);

    psx_bus_gpu_gp0_write(&bus, 0xe1001234u);
    assert(bus.gpu_draw_mode == 0x1234u && bus.gpu_gp0_write_count == 3u);
    psx_bus_gpu_gp0_write(&bus, 0xe6000003u);
    assert(bus.gpu_mask_setting == 3u);
    /* GPU ports are mapped as MMIO; GP1 commands update the status model. */
    assert(psx_bus_write32(&bus, 0x1f801810u, 0xe1004321u));
    assert(bus.gpu_draw_mode == 0x4321u);
    assert(psx_bus_write32(&bus, 0x1f801814u, 0x03000001u));
    assert(psx_bus_read32(&bus, 0x1f801814u, &value));
    assert((value & (1u << 23)) != 0u);
    assert(psx_bus_write32(&bus, 0x1f801814u, 0x03000000u));
    assert(psx_bus_read32(&bus, 0x1f801814u, &value));
    assert((value & (1u << 23)) == 0u);
    assert(bus.gpu_gp0_write_count == 5u);
    assert(bus.gpu_gp1_write_count == 2u);
    /* GP1 reset restores the modeled drawing environment and status. */
    bus.gpu_draw_mode = 0x1234u;
    bus.gpu_mask_setting = 3u;
    assert(psx_bus_write32(&bus, 0x1f801814u, 0x00000000u));
    assert(bus.gpu_draw_mode == 0u && bus.gpu_mask_setting == 0u);
    assert(psx_bus_read32(&bus, 0x1f801814u, &value) && value == 0x14802000u);
    /* Byte/halfword accesses to word-only GPU ports are rejected explicitly. */
    {
        uint8_t byte = 0u;
        uint16_t half = 0u;
        assert(!psx_bus_read8(&bus, 0x1f801810u, &byte));
        assert(!psx_bus_write8(&bus, 0x1f801810u, 0x12u));
        assert(!psx_bus_read16(&bus, 0x1f801814u, &half));
    }
    assert(!psx_bus_read32(&bus, 0x1f801000u, &value));

    puts("PSXBlaster360 bus/device tests: PASS");
    return 0;
}
