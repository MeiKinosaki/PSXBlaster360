#include "psx/memory.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    static psx_memory_t memory;
    uint8_t value8 = 0;
    uint16_t value16 = 0;
    uint32_t value32 = 0;
    static uint8_t bios_image[PSX_BIOS_SIZE];

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

    /* BIOS is optional, read-only, and visible through physical/KSEG aliases. */
    assert(!memory.bios_loaded);
    assert(!psx_memory_read32(&memory, 0xbfc00000u, &value32));
    assert(!psx_memory_load_bios(&memory, bios_image, PSX_BIOS_SIZE - 1u));
    memset(bios_image, 0, sizeof(bios_image));
    bios_image[0] = 0x78u;
    bios_image[1] = 0x56u;
    bios_image[2] = 0x34u;
    bios_image[3] = 0x12u;
    bios_image[PSX_BIOS_SIZE - 1u] = 0xabu;
    assert(psx_memory_load_bios(&memory, bios_image, PSX_BIOS_SIZE));
    assert(memory.bios_loaded);
    assert(psx_memory_read32(&memory, PSX_BIOS_BASE, &value32) && value32 == 0x12345678u);
    assert(psx_memory_read32(&memory, 0x9fc00000u, &value32) && value32 == 0x12345678u);
    assert(psx_memory_read32(&memory, 0xbfc00000u, &value32) && value32 == 0x12345678u);
    assert(psx_memory_read8(&memory, 0xbfc7ffffu, &value8) && value8 == 0xabu);
    assert(!psx_memory_write8(&memory, PSX_BIOS_BASE, 0u));
    assert(!psx_memory_write32(&memory, 0xbfc00000u, 0u));
    assert(!psx_memory_load_bios(&memory, NULL, PSX_BIOS_SIZE));

    puts("PSXBlaster360 memory tests: PASS");
    return 0;
}
