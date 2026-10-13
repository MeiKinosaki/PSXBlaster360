#include "psx/memory.h"

#include <stddef.h>
#include <string.h>

#define PSX_RAM_WINDOW_SIZE (8u * 1024u * 1024u)
#define PSX_KSEG0_BASE 0x80000000u
#define PSX_KSEG1_BASE 0xa0000000u

static bool ram_offset(uint32_t address, uint32_t *offset)
{
    uint32_t physical;

    if (offset == NULL) return false;

    /* Accept physical RAM window and its KSEG0/KSEG1 cached/uncached aliases. */
    if (address < PSX_RAM_WINDOW_SIZE) {
        physical = address;
    } else if (address >= PSX_KSEG0_BASE &&
               address < PSX_KSEG0_BASE + PSX_RAM_WINDOW_SIZE) {
        physical = address - PSX_KSEG0_BASE;
    } else if (address >= PSX_KSEG1_BASE &&
               address < PSX_KSEG1_BASE + PSX_RAM_WINDOW_SIZE) {
        physical = address - PSX_KSEG1_BASE;
    } else {
        return false;
    }

    *offset = physical & PSX_RAM_MIRROR_MASK;
    return true;
}

void psx_memory_init(psx_memory_t *memory)
{
    if (memory != NULL) {
        memset(memory->ram, 0, sizeof(memory->ram));
    }
}

bool psx_memory_read8(const psx_memory_t *memory, uint32_t address, uint8_t *value)
{
    uint32_t offset;
    if (memory == NULL || value == NULL || !ram_offset(address, &offset)) return false;
    *value = memory->ram[offset];
    return true;
}

bool psx_memory_read16(const psx_memory_t *memory, uint32_t address, uint16_t *value)
{
    uint32_t offset;
    if (memory == NULL || value == NULL || (address & 1u) != 0u ||
        !ram_offset(address, &offset)) return false;
    *value = (uint16_t)((uint16_t)memory->ram[offset] |
              ((uint16_t)memory->ram[(offset + 1u) & PSX_RAM_MIRROR_MASK] << 8));
    return true;
}

bool psx_memory_read32(const psx_memory_t *memory, uint32_t address, uint32_t *value)
{
    uint32_t offset;
    if (memory == NULL || value == NULL || (address & 3u) != 0u ||
        !ram_offset(address, &offset)) return false;
    *value = (uint32_t)memory->ram[offset] |
             ((uint32_t)memory->ram[(offset + 1u) & PSX_RAM_MIRROR_MASK] << 8) |
             ((uint32_t)memory->ram[(offset + 2u) & PSX_RAM_MIRROR_MASK] << 16) |
             ((uint32_t)memory->ram[(offset + 3u) & PSX_RAM_MIRROR_MASK] << 24);
    return true;
}

bool psx_memory_write8(psx_memory_t *memory, uint32_t address, uint8_t value)
{
    uint32_t offset;
    if (memory == NULL || !ram_offset(address, &offset)) return false;
    memory->ram[offset] = value;
    return true;
}

bool psx_memory_write16(psx_memory_t *memory, uint32_t address, uint16_t value)
{
    uint32_t offset;
    if (memory == NULL || (address & 1u) != 0u || !ram_offset(address, &offset)) return false;
    memory->ram[offset] = (uint8_t)(value & 0xffu);
    memory->ram[(offset + 1u) & PSX_RAM_MIRROR_MASK] = (uint8_t)(value >> 8);
    return true;
}

bool psx_memory_write32(psx_memory_t *memory, uint32_t address, uint32_t value)
{
    uint32_t offset;
    unsigned int i;
    if (memory == NULL || (address & 3u) != 0u || !ram_offset(address, &offset)) return false;
    for (i = 0; i < 4u; ++i) {
        memory->ram[(offset + i) & PSX_RAM_MIRROR_MASK] =
            (uint8_t)((value >> (8u * i)) & 0xffu);
    }
    return true;
}
