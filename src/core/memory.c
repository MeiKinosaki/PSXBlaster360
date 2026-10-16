#include "psx/memory.h"

#include <stddef.h>
#include <string.h>

#define PSX_RAM_WINDOW_SIZE (8u * 1024u * 1024u)
#define PSX_KSEG0_BASE 0x80000000u
#define PSX_KSEG1_BASE 0xa0000000u
#define PSX_KSEG0_PHYS_BIOS 0x9fc00000u
#define PSX_KSEG1_PHYS_BIOS 0xbfc00000u

static bool ram_offset(uint32_t address, uint32_t *offset)
{
    uint32_t physical;

    if (offset == NULL) return false;
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

static bool bios_offset(uint32_t address, uint32_t *offset)
{
    uint32_t physical;
    if (offset == NULL) return false;

    if (address >= PSX_BIOS_BASE && address < PSX_BIOS_BASE + PSX_BIOS_SIZE) {
        physical = address;
    } else if (address >= PSX_KSEG0_PHYS_BIOS &&
               address < PSX_KSEG0_PHYS_BIOS + PSX_BIOS_SIZE) {
        physical = address - PSX_KSEG0_BASE;
    } else if (address >= PSX_KSEG1_PHYS_BIOS &&
               address < PSX_KSEG1_PHYS_BIOS + PSX_BIOS_SIZE) {
        physical = address - PSX_KSEG1_BASE;
    } else {
        return false;
    }
    *offset = physical - PSX_BIOS_BASE;
    return true;
}

void psx_memory_init(psx_memory_t *memory)
{
    if (memory != NULL) {
        memset(memory->ram, 0, sizeof(memory->ram));
        memset(memory->bios, 0, sizeof(memory->bios));
        memory->bios_loaded = false;
    }
}

bool psx_memory_load_bios(psx_memory_t *memory, const uint8_t *image, uint32_t size)
{
    if (memory == NULL || image == NULL || size != PSX_BIOS_SIZE) return false;
    memcpy(memory->bios, image, PSX_BIOS_SIZE);
    memory->bios_loaded = true;
    return true;
}

bool psx_memory_read8(const psx_memory_t *memory, uint32_t address, uint8_t *value)
{
    uint32_t offset;
    if (memory == NULL || value == NULL) return false;
    if (bios_offset(address, &offset)) {
        if (!memory->bios_loaded) return false;
        *value = memory->bios[offset];
        return true;
    }
    if (!ram_offset(address, &offset)) return false;
    *value = memory->ram[offset];
    return true;
}

bool psx_memory_read16(const psx_memory_t *memory, uint32_t address, uint16_t *value)
{
    uint32_t offset;
    if (memory == NULL || value == NULL || (address & 1u) != 0u) return false;
    if (bios_offset(address, &offset)) {
        if (!memory->bios_loaded || offset + 1u >= PSX_BIOS_SIZE) return false;
        *value = (uint16_t)((uint16_t)memory->bios[offset] |
                  ((uint16_t)memory->bios[offset + 1u] << 8));
        return true;
    }
    if (!ram_offset(address, &offset)) return false;
    *value = (uint16_t)((uint16_t)memory->ram[offset] |
              ((uint16_t)memory->ram[(offset + 1u) & PSX_RAM_MIRROR_MASK] << 8));
    return true;
}

bool psx_memory_read32(const psx_memory_t *memory, uint32_t address, uint32_t *value)
{
    uint32_t offset;
    if (memory == NULL || value == NULL || (address & 3u) != 0u) return false;
    if (bios_offset(address, &offset)) {
        if (!memory->bios_loaded || offset + 3u >= PSX_BIOS_SIZE) return false;
        *value = (uint32_t)memory->bios[offset] |
                 ((uint32_t)memory->bios[offset + 1u] << 8) |
                 ((uint32_t)memory->bios[offset + 2u] << 16) |
                 ((uint32_t)memory->bios[offset + 3u] << 24);
        return true;
    }
    if (!ram_offset(address, &offset)) return false;
    *value = (uint32_t)memory->ram[offset] |
             ((uint32_t)memory->ram[(offset + 1u) & PSX_RAM_MIRROR_MASK] << 8) |
             ((uint32_t)memory->ram[(offset + 2u) & PSX_RAM_MIRROR_MASK] << 16) |
             ((uint32_t)memory->ram[(offset + 3u) & PSX_RAM_MIRROR_MASK] << 24);
    return true;
}

bool psx_memory_write8(psx_memory_t *memory, uint32_t address, uint8_t value)
{
    uint32_t offset;
    if (memory == NULL || bios_offset(address, &offset) || !ram_offset(address, &offset))
        return false;
    memory->ram[offset] = value;
    return true;
}

bool psx_memory_write16(psx_memory_t *memory, uint32_t address, uint16_t value)
{
    uint32_t offset;
    if (memory == NULL || (address & 1u) != 0u ||
        bios_offset(address, &offset) || !ram_offset(address, &offset)) return false;
    memory->ram[offset] = (uint8_t)(value & 0xffu);
    memory->ram[(offset + 1u) & PSX_RAM_MIRROR_MASK] = (uint8_t)(value >> 8);
    return true;
}

bool psx_memory_write32(psx_memory_t *memory, uint32_t address, uint32_t value)
{
    uint32_t offset;
    unsigned int i;
    if (memory == NULL || (address & 3u) != 0u ||
        bios_offset(address, &offset) || !ram_offset(address, &offset)) return false;
    for (i = 0; i < 4u; ++i) {
        memory->ram[(offset + i) & PSX_RAM_MIRROR_MASK] =
            (uint8_t)((value >> (8u * i)) & 0xffu);
    }
    return true;
}
