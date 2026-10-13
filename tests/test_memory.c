#include "psx/memory.h"

#include <assert.h>
#include <stdio.h>

int main(void)
{
    static psx_memory_t memory;
    uint8_t value8 = 0;
    uint16_t value16 = 0;
    uint32_t value32 = 0;

    psx_memory_init(&memory);
    assert(psx_memory_read32(&memory, 0, &value32) && value32 == 0);

    assert(psx_memory_write8(&memory, 0x10u, 0x5au));
    assert(psx_memory_read8(&memory, 0x10u, &value8) && value8 == 0x5au);
    assert(psx_memory_read8(&memory, 0x10u + PSX_RAM_SIZE, &value8) && value8 == 0x5au);
    assert(psx_memory_read8(&memory, 0x80000010u, &value8) && value8 == 0x5au);
    assert(psx_memory_read8(&memory, 0xa0000010u, &value8) && value8 == 0x5au);

    assert(psx_memory_write16(&memory, 0x20u, 0x1234u));
    assert(psx_memory_read16(&memory, 0x20u, &value16) && value16 == 0x1234u);
    assert(!psx_memory_read16(&memory, 0x21u, &value16));
    assert(!psx_memory_write16(&memory, 0x21u, 0x1234u));

    assert(psx_memory_write32(&memory, 0x100u, 0x89abcdefu));
    assert(psx_memory_read32(&memory, 0x100u, &value32) && value32 == 0x89abcdefu);
    assert(!psx_memory_read32(&memory, 0x102u, &value32));
    assert(!psx_memory_write32(&memory, 0x102u, 0x12345678u));

    assert(psx_memory_write32(&memory, PSX_RAM_SIZE - 4u, 0xa1b2c3d4u));
    assert(psx_memory_read32(&memory, PSX_RAM_SIZE - 4u, &value32) && value32 == 0xa1b2c3d4u);
    assert(psx_memory_read32(&memory, 0x007ffffcu, &value32) && value32 == 0xa1b2c3d4u);

    /* Non-RAM regions must not alias into RAM accidentally. */
    assert(!psx_memory_read8(&memory, 0x1f801000u, &value8));
    assert(!psx_memory_write8(&memory, 0x1f801000u, 0x77u));
    assert(!psx_memory_read8(&memory, 0x80800000u, &value8));
    assert(!psx_memory_read8(&memory, 0xfffffffcu, &value8));

    assert(!psx_memory_read8(NULL, 0, &value8));
    assert(!psx_memory_read8(&memory, 0, NULL));
    assert(!psx_memory_write8(NULL, 0, 0));

    puts("PSXBlaster360 memory tests: PASS");
    return 0;
}
