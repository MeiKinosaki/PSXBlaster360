#ifndef PSX_SYSTEM_H
#define PSX_SYSTEM_H

#include <stdbool.h>
#include <stdint.h>
#include "psx/bus.h"
#include "psx/cpu.h"
#include "psx/memory.h"

typedef struct psx_system {
    psx_memory_t memory;
    psx_cpu_t cpu;
    psx_bus_t bus;
    uint64_t cycles;
} psx_system_t;

/* Initializes the CPU/RAM/device state; this does not load a BIOS image. */
void psx_system_init(psx_system_t *system);
/* Execute one CPU instruction and advance the simplified device clock on success. */
psx_cpu_step_result_t psx_system_step(psx_system_t *system, uint32_t device_cycles);

#endif
