#include "psx/cpu.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static uint32_t i_type(uint32_t op, uint32_t rs, uint32_t rt, uint16_t imm)
{
    return (op << 26) | (rs << 21) | (rt << 16) | imm;
}

static uint32_t r_type(uint32_t rs, uint32_t rt, uint32_t rd, uint32_t shamt, uint32_t funct)
{
    return (rs << 21) | (rt << 16) | (rd << 11) | (shamt << 6) | funct;
}

int main(void)
{
    static psx_memory_t memory;
    psx_cpu_t cpu;

    psx_memory_init(&memory);
    psx_cpu_reset(&cpu);
    cpu.pc = 0;
    cpu.next_pc = 4;

    /* addiu t0, zero, 7; addiu t1, zero, -2; addu t2,t0,t1 */
    assert(psx_memory_write32(&memory, 0, i_type(0x09, 0, 8, 7)));
    assert(psx_memory_write32(&memory, 4, i_type(0x09, 0, 9, 0xfffe)));
    assert(psx_memory_write32(&memory, 8, r_type(8, 9, 10, 0, 0x21)));
    assert(psx_cpu_step(&cpu, &memory) == PSX_CPU_STEP_OK);
    assert(cpu.gpr[8] == 7u && cpu.pc == 4u);
    assert(psx_cpu_step(&cpu, &memory) == PSX_CPU_STEP_OK);
    assert(cpu.gpr[9] == 0xfffffffeu);
    assert(psx_cpu_step(&cpu, &memory) == PSX_CPU_STEP_OK);
    assert(cpu.gpr[10] == 5u);

    /* LUI/ORI and store/load round trip. */
    assert(psx_memory_write32(&memory, 12, i_type(0x0f, 0, 11, 0x1234)));
    assert(psx_memory_write32(&memory, 16, i_type(0x0d, 11, 11, 0x5678)));
    assert(psx_memory_write32(&memory, 20, i_type(0x2b, 0, 11, 0x200)));
    assert(psx_memory_write32(&memory, 24, i_type(0x23, 0, 12, 0x200)));
    assert(psx_cpu_step(&cpu, &memory) == PSX_CPU_STEP_OK);
    assert(psx_cpu_step(&cpu, &memory) == PSX_CPU_STEP_OK);
    assert(cpu.gpr[11] == 0x12345678u);
    assert(psx_cpu_step(&cpu, &memory) == PSX_CPU_STEP_OK);
    assert(psx_cpu_step(&cpu, &memory) == PSX_CPU_STEP_OK);
    assert(cpu.gpr[12] == 0x12345678u);

    /* BEQ takes effect after exactly one delay-slot instruction. */
    assert(psx_memory_write32(&memory, 28, i_type(0x04, 8, 8, 2)));
    assert(psx_memory_write32(&memory, 32, i_type(0x09, 0, 13, 99)));
    assert(psx_memory_write32(&memory, 36, i_type(0x09, 0, 14, 11)));
    assert(psx_memory_write32(&memory, 40, i_type(0x09, 0, 15, 22)));
    assert(psx_cpu_step(&cpu, &memory) == PSX_CPU_STEP_OK);
    assert(cpu.pc == 32u && cpu.next_pc == 40u);
    assert(psx_cpu_step(&cpu, &memory) == PSX_CPU_STEP_OK);
    assert(cpu.gpr[13] == 99u && cpu.pc == 40u);
    assert(psx_cpu_step(&cpu, &memory) == PSX_CPU_STEP_OK);
    assert(cpu.gpr[14] == 0u && cpu.gpr[15] == 22u);

    /* Unsupported opcodes stop without counting an instruction. */
    assert(psx_memory_write32(&memory, 44, 0xffffffffu));
    {
        uint64_t retired = cpu.instructions_retired;
        cpu.pc = 44; cpu.next_pc = 48;
        assert(psx_cpu_step(&cpu, &memory) == PSX_CPU_STEP_UNSUPPORTED_INSTRUCTION);
        assert(cpu.instructions_retired == retired);
        assert(cpu.pc == 44u);
    }

    assert(psx_cpu_sign_extend16(0x8000u) == -32768);
    assert(psx_cpu_sign_extend8(0xffu) == -1);
    puts("PSXBlaster360 CPU execution tests: PASS");
    return 0;
}
