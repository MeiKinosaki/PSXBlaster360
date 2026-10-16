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
    assert(psx_bus_write32(&bus, 0x1f8010f0u, 0x07654321u));
    assert(psx_bus_read32(&bus, 0x1f8010f0u, &value) && value == 0x07654321u);

    psx_bus_gpu_gp0_write(&bus, 0xe1001234u);
    assert(bus.gpu_draw_mode == 0x1234u && bus.gpu_gp0_write_count == 1u);
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
    assert(bus.gpu_gp0_write_count == 3u);
    assert(bus.gpu_gp1_write_count == 2u);
    assert(!psx_bus_read32(&bus, 0x1f801000u, &value));

    puts("PSXBlaster360 bus/device tests: PASS");
    return 0;
}
