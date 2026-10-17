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

    /* External IRQ reaches COP0 only when Status.IEc and an IP mask are set. */
    system.cpu.pc = 0x40u;
    system.cpu.next_pc = 0x44u;
    system.cpu.cop0_status = (1u << 10) | 1u; /* IM2 + global interrupt enable */
    system.cpu.cop0_cause = 0u;
    system.bus.irq_mask = 1u << 4;
    system.bus.irq_status = 1u << 4;
    assert(psx_memory_write32(&system.memory, 0x40u, 0x24020007u));
    assert(psx_system_step(&system, 1u) == PSX_CPU_STEP_EXCEPTION);
    assert(system.cpu.cop0_epc == 0x40u);
    assert(((system.cpu.cop0_cause >> 2) & 31u) == 0u);
    assert((system.cpu.cop0_cause & (1u << 10)) != 0u);
    assert(system.cpu.pc == 0x80000080u);
    assert(system.cpu.gpr[2] == 5u); /* interrupted instruction was not retired */

    /* A pending IRQ is masked when global IE is clear. */
    system.cpu.pc = 0x44u;
    system.cpu.next_pc = 0x48u;
    system.cpu.cop0_status = 1u << 10;
    assert(psx_memory_write32(&system.memory, 0x44u, 0x24030009u));
    assert(psx_system_step(&system, 1u) == PSX_CPU_STEP_OK);
    assert(system.cpu.gpr[3] == 9u);

    /* A pending source outside I_MASK must not assert external IP2. */
    system.cpu.pc = 0x48u;
    system.cpu.next_pc = 0x4cu;
    system.cpu.cop0_status = (1u << 10) | 1u;
    system.bus.irq_mask = 0u;
    assert(psx_memory_write32(&system.memory, 0x48u, 0x240c000bu));
    assert(psx_system_step(&system, 1u) == PSX_CPU_STEP_OK);
    assert(system.cpu.gpr[12] == 11u);
    assert((system.cpu.cop0_cause & (1u << 10)) == 0u);

    /* Resume the sequential MMIO integration checks at their expected PC. */
    system.cpu.pc = 4u;
    system.cpu.next_pc = 8u;

    /* CPU SW reaches the GPU GP0 register through the integrated MMIO bus. */
    system.cpu.gpr[8] = 0x1f801810u; /* t0 = GP0 */
    system.cpu.gpr[9] = 0xe1002222u; /* t1 = GP0 draw-mode command */
    assert(psx_memory_write32(&system.memory, 4u, 0xad090000u)); /* SW t1,0(t0) */
    assert(psx_system_step(&system, 1u) == PSX_CPU_STEP_OK);
    assert(system.bus.gpu_draw_mode == 0x2222u);
    assert(system.bus.gpu_gp0_write_count == 1u);

    /* Byte store now travels through the integrated bus to RAM. */
    system.cpu.gpr[8] = 0x300u;
    system.cpu.gpr[9] = 0x80u;
    assert(psx_memory_write32(&system.memory, 8u, 0xa1090001u)); /* SB t1,1(t0) */
    assert(psx_system_step(&system, 1u) == PSX_CPU_STEP_OK);
    {
        uint8_t byte = 0u;
        assert(psx_memory_read8(&system.memory, 0x301u, &byte));
        assert(byte == 0x80u);
    }

    /* LBU/LHU in the integrated CPU path must use the bus accessors. */
    system.cpu.gpr[8] = 0x300u;
    assert(psx_memory_write8(&system.memory, 0x302u, 0xa5u));
    assert(psx_memory_write16(&system.memory, 0x304u, 0x9876u));
    assert(psx_memory_write32(&system.memory, 12u, 0x910a0002u)); /* LBU t2,2(t0) */
    assert(psx_memory_write32(&system.memory, 16u, 0x950b0004u)); /* LHU t3,4(t0) */
    assert(psx_system_step(&system, 1u) == PSX_CPU_STEP_OK);
    assert(system.cpu.gpr[10] == 0xa5u);
    assert(psx_system_step(&system, 1u) == PSX_CPU_STEP_OK);
    assert(system.cpu.gpr[11] == 0x9876u);

    /* LHU misalignment raises AdEL and records BadVAddr. */
    assert(psx_memory_write32(&system.memory, 20u, 0x950b0003u));
    assert(psx_system_step(&system, 1u) == PSX_CPU_STEP_EXCEPTION);
    assert(system.cpu.cop0_bad_vaddr == 0x303u);
    assert(((system.cpu.cop0_cause >> 2) & 31u) == 4u);

    /* SH also uses the integrated bus, while sub-word GPU MMIO stays blocked. */
    system.cpu.pc = 24u;
    system.cpu.next_pc = 28u;
    system.cpu.gpr[8] = 0x300u;
    system.cpu.gpr[11] = 0x1357u;
    assert(psx_memory_write32(&system.memory, 24u, 0xa50b0006u)); /* SH t3,6(t0) */
    assert(psx_system_step(&system, 1u) == PSX_CPU_STEP_OK);
    {
        uint16_t half = 0u;
        assert(psx_memory_read16(&system.memory, 0x306u, &half));
        assert(half == 0x1357u);
    }

    /* Halfword bus rejects MMIO sub-word access instead of corrupting a register. */
    assert(!psx_bus_write16(&system.bus, 0x1f801810u, 0x1234u));
    puts("PSXBlaster360 system integration tests: PASS");
    return 0;
}
