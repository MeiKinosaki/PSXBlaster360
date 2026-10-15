#include "psx/system.h"

#include <stddef.h>
#include <string.h>

void psx_system_init(psx_system_t *system)
{
    if (system == NULL) return;
    memset(system, 0, sizeof(*system));
    psx_memory_init(&system->memory);
    psx_cpu_reset(&system->cpu);
    psx_bus_init(&system->bus, &system->memory);
}

psx_cpu_step_result_t psx_system_step(psx_system_t *system, uint32_t device_cycles)
{
    psx_cpu_step_result_t result;
    if (system == NULL) return PSX_CPU_STEP_MEMORY_FAULT;
    result = psx_cpu_step(&system->cpu, &system->memory);
    if (result == PSX_CPU_STEP_OK) {
        psx_bus_tick(&system->bus, device_cycles);
        system->cycles += device_cycles;
    }
    return result;
}
