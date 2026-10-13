#ifndef PSX_MEMORY_H
#define PSX_MEMORY_H

#include <stdbool.h>
#include <stdint.h>

#define PSX_RAM_SIZE       (2u * 1024u * 1024u)
#define PSX_RAM_MIRROR_MASK (PSX_RAM_SIZE - 1u)

typedef struct psx_memory {
    uint8_t ram[PSX_RAM_SIZE];
} psx_memory_t;

/* Initializes all emulated main RAM bytes to zero. */
void psx_memory_init(psx_memory_t *memory);

/* RAM-only accessors. Return false for null pointers or misaligned accesses. */
bool psx_memory_read8(const psx_memory_t *memory, uint32_t address, uint8_t *value);
bool psx_memory_read16(const psx_memory_t *memory, uint32_t address, uint16_t *value);
bool psx_memory_read32(const psx_memory_t *memory, uint32_t address, uint32_t *value);
bool psx_memory_write8(psx_memory_t *memory, uint32_t address, uint8_t value);
bool psx_memory_write16(psx_memory_t *memory, uint32_t address, uint16_t value);
bool psx_memory_write32(psx_memory_t *memory, uint32_t address, uint32_t value);

#endif
