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
    uint32_t value = 0u;

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
        assert(psx_cpu_step(&cpu, &memory) == PSX_CPU_STEP_EXCEPTION);
        assert(cpu.instructions_retired == retired);
        assert(cpu.cop0_epc == 44u);
        assert(((cpu.cop0_cause >> 2) & 31u) == 10u);
        assert(cpu.pc == 0x80000080u);
    }

    /* Byte/halfword sign extension and stores use the same RAM bus. */
    cpu.pc = 64u; cpu.next_pc = 68u;
    cpu.gpr[1] = 0x300u;
    assert(psx_memory_write8(&memory, 0x301u, 0x80u));
    assert(psx_memory_write16(&memory, 0x304u, 0x8001u));
    assert(psx_memory_write32(&memory, 64, i_type(0x20, 1, 2, 1)));  /* LB v0,1(at) */
    assert(psx_memory_write32(&memory, 68, i_type(0x24, 1, 3, 1)));  /* LBU v1,1(at) */
    assert(psx_memory_write32(&memory, 72, i_type(0x21, 1, 4, 4)));  /* LH a0,4(at) */
    assert(psx_memory_write32(&memory, 76, i_type(0x25, 1, 5, 4)));  /* LHU a1,4(at) */
    assert(psx_memory_write32(&memory, 80, i_type(0x28, 1, 2, 8)));  /* SB v0,8(at) */
    assert(psx_memory_write32(&memory, 84, i_type(0x29, 1, 5, 10))); /* SH a1,10(at) */
    assert(psx_cpu_step(&cpu, &memory) == PSX_CPU_STEP_OK);
    assert(cpu.gpr[2] == 0xffffff80u);
    assert(psx_cpu_step(&cpu, &memory) == PSX_CPU_STEP_OK);
    assert(cpu.gpr[3] == 0x80u);
    assert(psx_cpu_step(&cpu, &memory) == PSX_CPU_STEP_OK);
    assert(cpu.gpr[4] == 0xffff8001u);
    assert(psx_cpu_step(&cpu, &memory) == PSX_CPU_STEP_OK);
    assert(cpu.gpr[5] == 0x8001u);
    assert(psx_cpu_step(&cpu, &memory) == PSX_CPU_STEP_OK);
    { uint8_t byte = 0; assert(psx_memory_read8(&memory, 0x308u, &byte) && byte == 0x80u); }
    assert(psx_cpu_step(&cpu, &memory) == PSX_CPU_STEP_OK);
    { uint16_t half = 0; assert(psx_memory_read16(&memory, 0x30au, &half) && half == 0x8001u); }

    /* LWL/LWR merge bytes from adjacent aligned words. */
    psx_cpu_reset(&cpu);
    cpu.pc = 0u; cpu.next_pc = 4u; cpu.gpr[1] = 0x400u;
    cpu.gpr[2] = 0xaabbccddu;
    assert(psx_memory_write32(&memory, 0x400u, 0x11223344u));
    assert(psx_memory_write32(&memory, 0x404u, 0x55667788u));
    assert(psx_memory_write32(&memory, 0u, i_type(0x22, 1, 2, 1))); /* LWL v0,1(at) */
    assert(psx_memory_write32(&memory, 4u, i_type(0x26, 1, 2, 2))); /* LWR v0,2(at) */
    assert(psx_cpu_step(&cpu, &memory) == PSX_CPU_STEP_OK);
    assert(cpu.gpr[2] == 0x223344ddu);
    assert(psx_cpu_step(&cpu, &memory) == PSX_CPU_STEP_OK);
    assert(cpu.gpr[2] == 0x77885566u);

    /* SWL/SWR update only the addressed byte lanes of the aligned word. */
    psx_cpu_reset(&cpu);
    cpu.pc = 0u; cpu.next_pc = 4u; cpu.gpr[1] = 0x420u; cpu.gpr[2] = 0xa1b2c3d4u;
    assert(psx_memory_write32(&memory, 0x420u, 0x11223344u));
    assert(psx_memory_write32(&memory, 0u, i_type(0x2a, 1, 2, 1))); /* SWL v0,1(at) */
    assert(psx_memory_write32(&memory, 4u, i_type(0x2e, 1, 2, 2))); /* SWR v0,2(at) */
    assert(psx_cpu_step(&cpu, &memory) == PSX_CPU_STEP_OK);
    assert(psx_memory_read32(&memory, 0x420u, &value));
    assert(value == 0x1122a1b2u);
    assert(psx_cpu_step(&cpu, &memory) == PSX_CPU_STEP_OK);
    assert(psx_memory_read32(&memory, 0x420u, &value));
    assert(value == 0xc3d4a1b2u);

    /* Unaligned halfword access must fault without retiring. */
    cpu.pc = 88u; cpu.next_pc = 92u;
    cpu.gpr[1] = 0x300u;
    assert(psx_memory_write32(&memory, 88, i_type(0x21, 1, 6, 1)));
    {
        uint64_t retired = cpu.instructions_retired;
        assert(psx_cpu_step(&cpu, &memory) == PSX_CPU_STEP_EXCEPTION);
        assert(cpu.instructions_retired == retired);
        assert(cpu.cop0_bad_vaddr == 0x301u);
        assert(((cpu.cop0_cause >> 2) & 31u) == 4u);
    }

    /* Signed overflow raises COP0 exception without retiring the instruction. */
    psx_cpu_reset(&cpu);
    cpu.pc = 0u; cpu.next_pc = 4u;
    cpu.gpr[1] = 0x7fffffffu;
    assert(psx_memory_write32(&memory, 0, i_type(0x08, 1, 2, 1))); /* ADDI v0,at,1 */
    {
        uint64_t retired = cpu.instructions_retired;
        assert(psx_cpu_step(&cpu, &memory) == PSX_CPU_STEP_EXCEPTION);
        assert(((cpu.cop0_cause >> 2) & 31u) == 12u);
        assert(cpu.cop0_epc == 0u);
        assert(cpu.gpr[2] == 0u);
        assert(cpu.pc == 0x80000080u);
        assert(cpu.instructions_retired == retired);
    }

    /* SYSCALL raises exception code 8 and records EPC. */
    psx_cpu_reset(&cpu);
    cpu.pc = 0u; cpu.next_pc = 4u;
    assert(psx_memory_write32(&memory, 0, 0x0000000cu));
    assert(psx_cpu_step(&cpu, &memory) == PSX_CPU_STEP_EXCEPTION);
    assert(((cpu.cop0_cause >> 2) & 31u) == 8u);
    assert(cpu.cop0_epc == 0u);

    /* Exceptions in a branch delay slot set BD and use the branch PC as EPC. */
    psx_cpu_reset(&cpu);
    cpu.pc = 0u; cpu.next_pc = 4u;
    assert(psx_memory_write32(&memory, 0, i_type(0x04, 0, 0, 1))); /* BEQ zero,zero,+1 */
    assert(psx_memory_write32(&memory, 4, 0x0000000cu)); /* SYSCALL in delay slot */
    assert(psx_cpu_step(&cpu, &memory) == PSX_CPU_STEP_OK);
    assert(cpu.branch_delay_pending == 1u);
    assert(psx_cpu_step(&cpu, &memory) == PSX_CPU_STEP_EXCEPTION);
    assert(cpu.cop0_epc == 0u);
    assert((cpu.cop0_cause & 0x80000000u) != 0u);
    assert(((cpu.cop0_cause >> 2) & 31u) == 8u);

    /* COP0 moves, RFE status stack and address-error exception. */
    psx_cpu_reset(&cpu);
    cpu.pc = 0u; cpu.next_pc = 4u;
    cpu.cop0_status = 0x0000003cu;
    assert(psx_memory_write32(&memory, 0, (0x10u << 26) | (0x00u << 21) | (2u << 16) | (12u << 11))); /* MFC0 v0,Status */
    assert(psx_memory_write32(&memory, 4, (0x10u << 26) | (0x04u << 21) | (3u << 16) | (12u << 11))); /* MTC0 v1,Status */
    assert(psx_memory_write32(&memory, 8, (0x10u << 26) | (0x10u << 21) | 0x10u)); /* RFE */
    assert(psx_cpu_step(&cpu, &memory) == PSX_CPU_STEP_OK);
    assert(cpu.gpr[2] == 0x3cu);
    cpu.gpr[3] = 0x3cu;
    assert(psx_cpu_step(&cpu, &memory) == PSX_CPU_STEP_OK);
    assert(cpu.cop0_status == 0x3cu);
    assert(psx_cpu_step(&cpu, &memory) == PSX_CPU_STEP_OK);
    assert(cpu.cop0_status == 0x3fu);

    psx_cpu_reset(&cpu);
    cpu.pc = 0u; cpu.next_pc = 4u; cpu.gpr[1] = 0x301u;
    assert(psx_memory_write32(&memory, 0, i_type(0x21, 1, 2, 0))); /* LH v0,0(at) unaligned */
    assert(psx_cpu_step(&cpu, &memory) == PSX_CPU_STEP_EXCEPTION);
    assert(((cpu.cop0_cause >> 2) & 31u) == 4u);
    assert(cpu.cop0_bad_vaddr == 0x301u);

    /* SLTI is signed while SLTIU compares against sign-extended immediate unsigned. */
    psx_cpu_reset(&cpu);
    cpu.pc = 0u; cpu.next_pc = 4u; cpu.gpr[1] = 0u;
    assert(psx_memory_write32(&memory, 0, i_type(0x0a, 1, 2, 0xffff))); /* SLTI v0,at,-1 */
    assert(psx_memory_write32(&memory, 4, i_type(0x0b, 1, 3, 0xffff))); /* SLTIU v1,at,-1 */
    assert(psx_cpu_step(&cpu, &memory) == PSX_CPU_STEP_OK);
    assert(cpu.gpr[2] == 0u);
    assert(psx_cpu_step(&cpu, &memory) == PSX_CPU_STEP_OK);
    assert(cpu.gpr[3] == 1u);

    /* Variable shifts and signed/unsigned division use HI/LO correctly. */
    psx_cpu_reset(&cpu);
    cpu.pc = 0u; cpu.next_pc = 4u;
    cpu.gpr[1] = 3u; cpu.gpr[2] = 0x10u;
    assert(psx_memory_write32(&memory, 0, r_type(1, 2, 3, 0, 0x04))); /* SLLV v1,v0,at */
    assert(psx_memory_write32(&memory, 4, r_type(0, 2, 0, 0, 0x1a))); /* DIV zero,v0 */
    assert(psx_memory_write32(&memory, 8, r_type(0, 2, 0, 0, 0x1b))); /* DIVU zero,v0 */
    assert(psx_cpu_step(&cpu, &memory) == PSX_CPU_STEP_OK);
    assert(cpu.gpr[3] == 0x80u);
    cpu.gpr[1] = 100u; cpu.gpr[2] = 7u;
    assert(psx_memory_write32(&memory, 12, r_type(1, 2, 0, 0, 0x1b)));
    cpu.pc = 12u; cpu.next_pc = 16u;
    assert(psx_cpu_step(&cpu, &memory) == PSX_CPU_STEP_OK);
    assert(cpu.lo == 14u && cpu.hi == 2u);

    assert(psx_cpu_sign_extend16(0x8000u) == -32768);
    assert(psx_cpu_sign_extend8(0xffu) == -1);
    puts("PSXBlaster360 CPU execution tests: PASS");
    return 0;
}
