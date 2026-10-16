#include "psx/system.h"

#include <assert.h>
#include <stdio.h>
#include <stdint.h>

int main(void)
{
    psx_system_t system;
    static uint8_t bios[PSX_BIOS_SIZE];
    psx_system_init(&system);
    assert(system.cpu.pc == 0xbfc00000u);
    assert(system.cycles == 0u);
    /* A user-supplied BIOS image is fetchable from the reset vector. */
    assert(psx_memory_load_bios(&system.memory, bios, PSX_BIOS_SIZE));
    assert(psx_system_step(&system, 1u) == PSX_CPU_STEP_OK);
    assert(system.cpu.pc == 0xbfc00004u);
    psx_system_init(&system);
    /* Test the integrated scheduler on RAM after resetting the test machine. */
    system.cpu.pc = 0u;
    system.cpu.next_pc = 4u;
    assert(psx_memory_write32(&system.memory, 0u, 0x24020005u)); /* ADDIU v0,zero,5 */
    system.bus.timers[0].target = 10u;
    system.bus.timers[0].mode = 1u << 4;
    assert(psx_system_step(&system, 12u) == PSX_CPU_STEP_OK);
    assert(system.cpu.gpr[2] == 5u);
    assert(system.cycles == 12u);
    assert((system.bus.irq_status & (1u << 4)) != 0u);
    puts("PSXBlaster360 system integration tests: PASS");
    return 0;
}
