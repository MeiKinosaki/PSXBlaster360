#include "psx/memory.h"

#include <stddef.h>
#include <string.h>

static uint32_t ram_offset(uint32_t address)
{
    /* PS1 main RAM is 2 MiB and is mirrored through its RAM address window. */
    return address & PSX_RAM_MIRROR_MASK;
}

void psx_memory_init(psx_memory_t *memory)
{
    if (memory != NULL) {
        memset(memory->ram, 0, sizeof(memory->ram));
    }
}

bool psx_memory_read8(const psx_memory_t *memory, uint32_t address, uint8_t *value)
{
    if (memory == NULL || value == NULL) return false;
    *value = memory->ram[ram_offset(address)];
    return true;
}

bool psx_memory_read16(const psx_memory_t *memory, uint32_t address, uint16_t *value)
{
    uint32_t offset;
    if (memory == NULL || value == NULL || (address & 1u) != 0u) return false;
    offset = ram_offset(address);
    *value = (uint16_t)((uint16_t)memory->ram[offset] |
              ((uint16_t)memory->ram[(offset + 1u) & PSX_RAM_MIRROR_MASK] << 8));
    return true;
}

bool psx_memory_read32(const psx_memory_t *memory, uint32_t address, uint32_t *value)
{
    uint32_t offset;
    if (memory == NULL || value == NULL || (address & 3u) != 0u) return false;
    offset = ram_offset(address);
    *value = (uint32_t)memory->ram[offset] |
             ((uint32_t)memory->ram[(offset + 1u) & PSX_RAM_MIRROR_MASK] << 8) |
             ((uint32_t)memory->ram[(offset + 2u) & PSX_RAM_MIRROR_MASK] << 16) |
             ((uint32_t)memory->ram[(offset + 3u) & PSX_RAM_MIRROR_MASK] << 24);
    return true;
}

bool psx_memory_write8(psx_memory_t *memory, uint32_t address, uint8_t value)
{
    if (memory == NULL) return false;
    memory->ram[ram_offset(address)] = value;
    return true;
}

bool psx_memory_write16(psx_memory_t *memory, uint32_t address, uint16_t value)
{
    uint32_t offset;
    if (memory == NULL || (address & 1u) != 0u) return false;
    offset = ram_offset(address);
    memory->ram[offset] = (uint8_t)(value & 0xffu);
    memory->ram[(offset + 1u) & PSX_RAM_MIRROR_MASK] = (uint8_t)(value >> 8);
    return true;
}

bool psx_memory_write32(psx_memory_t *memory, uint32_t address, uint32_t value)
{
    uint32_t offset;
    unsigned int i;
    if (memory == NULL || (address & 3u) != 0u) return false;
    offset = ram_offset(address);
    for (i = 0; i < 4u; ++i) {
        memory->ram[(offset + i) & PSX_RAM_MIRROR_MASK] =
            (uint8_t)((value >> (8u * i)) & 0xffu);
    }
    return true;
}
