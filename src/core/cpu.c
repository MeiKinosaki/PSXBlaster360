#include "psx/cpu.h"

#include <stddef.h>
#include <string.h>

void psx_cpu_reset(psx_cpu_t *cpu)
{
    if (cpu == NULL) return;
    memset(cpu, 0, sizeof(*cpu));
    cpu->pc = 0xbfc00000u;
    cpu->next_pc = 0xbfc00004u;
    cpu->cop0_status = 0x10900000u;
}

int32_t psx_cpu_sign_extend16(uint16_t immediate)
{
    return (int32_t)(int16_t)immediate;
}

int32_t psx_cpu_sign_extend8(uint8_t immediate)
{
    return (int32_t)(int8_t)immediate;
}

static uint32_t branch_target(uint32_t pc_after_instruction, uint16_t immediate)
{
    return pc_after_instruction + ((uint32_t)psx_cpu_sign_extend16(immediate) << 2);
}

psx_cpu_step_result_t psx_cpu_step(psx_cpu_t *cpu, psx_memory_t *memory)
{
    uint32_t instruction, old_pc, rs, rt, rd, shamt, opcode, funct, address;
    uint16_t imm;
    uint32_t a, b, result;
    int32_t signed_a, signed_b;
    if (cpu == NULL || memory == NULL) return PSX_CPU_STEP_MEMORY_FAULT;

    old_pc = cpu->pc;
    if (!psx_memory_read32(memory, old_pc, &instruction))
        return PSX_CPU_STEP_MEMORY_FAULT;

    opcode = instruction >> 26;
    rs = (instruction >> 21) & 31u;
    rt = (instruction >> 16) & 31u;
    rd = (instruction >> 11) & 31u;
    shamt = (instruction >> 6) & 31u;
    funct = instruction & 63u;
    imm = (uint16_t)instruction;
    a = cpu->gpr[rs];
    b = cpu->gpr[rt];
    signed_a = (int32_t)a;
    signed_b = (int32_t)b;

    /* Advance first; branches overwrite next_pc, preserving the delay slot. */
    cpu->pc = cpu->next_pc;
    cpu->next_pc += 4u;
    result = 0u;

    switch (opcode) {
    case 0x00u: /* SPECIAL */
        switch (funct) {
        case 0x00u: result = b << shamt; if (rd) cpu->gpr[rd] = result; break; /* SLL */
        case 0x02u: result = b >> shamt; if (rd) cpu->gpr[rd] = result; break; /* SRL */
        case 0x03u: result = (uint32_t)(signed_b >> shamt); if (rd) cpu->gpr[rd] = result; break; /* SRA */
        case 0x08u: cpu->next_pc = a; break; /* JR */
        case 0x09u: if (rd) cpu->gpr[rd] = old_pc + 8u; cpu->next_pc = a; break; /* JALR */
        case 0x10u: if (rd) cpu->gpr[rd] = cpu->hi; break; /* MFHI */
        case 0x12u: if (rd) cpu->gpr[rd] = cpu->lo; break; /* MFLO */
        case 0x18u: { /* MULT signed */
            int64_t product = (int64_t)signed_a * (int64_t)signed_b;
            cpu->lo = (uint32_t)product; cpu->hi = (uint32_t)((uint64_t)product >> 32); break;
        }
        case 0x19u: { /* MULTU */
            uint64_t product = (uint64_t)a * (uint64_t)b;
            cpu->lo = (uint32_t)product; cpu->hi = (uint32_t)(product >> 32); break;
        }
        case 0x20u: result = a + b; if (rd) cpu->gpr[rd] = result; break; /* ADD (overflow trap deferred) */
        case 0x21u: result = a + b; if (rd) cpu->gpr[rd] = result; break; /* ADDU */
        case 0x22u: result = a - b; if (rd) cpu->gpr[rd] = result; break; /* SUB (overflow trap deferred) */
        case 0x23u: result = a - b; if (rd) cpu->gpr[rd] = result; break; /* SUBU */
        case 0x24u: if (rd) cpu->gpr[rd] = a & b; break;
        case 0x25u: if (rd) cpu->gpr[rd] = a | b; break;
        case 0x26u: if (rd) cpu->gpr[rd] = a ^ b; break;
        case 0x27u: if (rd) cpu->gpr[rd] = ~(a | b); break;
        case 0x2au: if (rd) cpu->gpr[rd] = signed_a < signed_b ? 1u : 0u; break;
        case 0x2bu: if (rd) cpu->gpr[rd] = a < b ? 1u : 0u; break;
        default: cpu->pc = old_pc; cpu->next_pc = old_pc + 4u; return PSX_CPU_STEP_UNSUPPORTED_INSTRUCTION;
        }
        break;
    case 0x02u: /* J */
        cpu->next_pc = (cpu->pc & 0xf0000000u) | ((instruction & 0x03ffffffu) << 2);
        break;
    case 0x03u: /* JAL */
        cpu->gpr[31] = old_pc + 8u;
        cpu->next_pc = (cpu->pc & 0xf0000000u) | ((instruction & 0x03ffffffu) << 2);
        break;
    case 0x04u: if (a == b) cpu->next_pc = branch_target(old_pc + 4u, imm); break; /* BEQ */
    case 0x05u: if (a != b) cpu->next_pc = branch_target(old_pc + 4u, imm); break; /* BNE */
    case 0x06u: if (signed_a <= 0) cpu->next_pc = branch_target(old_pc + 4u, imm); break; /* BLEZ */
    case 0x07u: if (signed_a > 0) cpu->next_pc = branch_target(old_pc + 4u, imm); break; /* BGTZ */
    case 0x08u: result = a + (uint32_t)psx_cpu_sign_extend16(imm); if (rt) cpu->gpr[rt] = result; break; /* ADDI */
    case 0x09u: result = a + (uint32_t)psx_cpu_sign_extend16(imm); if (rt) cpu->gpr[rt] = result; break; /* ADDIU */
    case 0x0au: if (rt) cpu->gpr[rt] = signed_a < psx_cpu_sign_extend16(imm) ? 1u : 0u; break;
    case 0x0bu: if (rt) cpu->gpr[rt] = a < (uint32_t)psx_cpu_sign_extend16(imm) ? 1u : 0u; break;
    case 0x0cu: if (rt) cpu->gpr[rt] = a & (uint32_t)imm; break;
    case 0x0du: if (rt) cpu->gpr[rt] = a | (uint32_t)imm; break;
    case 0x0eu: if (rt) cpu->gpr[rt] = a ^ (uint32_t)imm; break;
    case 0x0fu: if (rt) cpu->gpr[rt] = (uint32_t)imm << 16; break;
    case 0x20u: /* LB */
        address = a + (uint32_t)psx_cpu_sign_extend16(imm);
        {
            uint8_t byte;
            if (!psx_memory_read8(memory, address, &byte)) { cpu->pc = old_pc; cpu->next_pc = old_pc + 4u; return PSX_CPU_STEP_MEMORY_FAULT; }
            result = (uint32_t)psx_cpu_sign_extend8(byte);
        }
        if (rt) cpu->gpr[rt] = result;
        break;
    case 0x21u: /* LH */
        address = a + (uint32_t)psx_cpu_sign_extend16(imm);
        {
            uint16_t half;
            if (!psx_memory_read16(memory, address, &half)) { cpu->pc = old_pc; cpu->next_pc = old_pc + 4u; return PSX_CPU_STEP_MEMORY_FAULT; }
            result = (uint32_t)(int32_t)(int16_t)half;
        }
        if (rt) cpu->gpr[rt] = result;
        break;
    case 0x24u: /* LBU */
        address = a + (uint32_t)psx_cpu_sign_extend16(imm);
        {
            uint8_t byte;
            if (!psx_memory_read8(memory, address, &byte)) { cpu->pc = old_pc; cpu->next_pc = old_pc + 4u; return PSX_CPU_STEP_MEMORY_FAULT; }
            result = byte;
        }
        if (rt) cpu->gpr[rt] = result;
        break;
    case 0x25u: /* LHU */
        address = a + (uint32_t)psx_cpu_sign_extend16(imm);
        {
            uint16_t half;
            if (!psx_memory_read16(memory, address, &half)) { cpu->pc = old_pc; cpu->next_pc = old_pc + 4u; return PSX_CPU_STEP_MEMORY_FAULT; }
            result = half;
        }
        if (rt) cpu->gpr[rt] = result;
        break;
    case 0x28u: /* SB */
        address = a + (uint32_t)psx_cpu_sign_extend16(imm);
        if (!psx_memory_write8(memory, address, (uint8_t)b)) { cpu->pc = old_pc; cpu->next_pc = old_pc + 4u; return PSX_CPU_STEP_MEMORY_FAULT; }
        break;
    case 0x29u: /* SH */
        address = a + (uint32_t)psx_cpu_sign_extend16(imm);
        if (!psx_memory_write16(memory, address, (uint16_t)b)) { cpu->pc = old_pc; cpu->next_pc = old_pc + 4u; return PSX_CPU_STEP_MEMORY_FAULT; }
        break;
    case 0x23u: /* LW */
        address = a + (uint32_t)psx_cpu_sign_extend16(imm);
        if (!psx_memory_read32(memory, address, &result)) { cpu->pc = old_pc; cpu->next_pc = old_pc + 4u; return PSX_CPU_STEP_MEMORY_FAULT; }
        if (rt) cpu->gpr[rt] = result;
        break;
    case 0x2bu: /* SW */
        address = a + (uint32_t)psx_cpu_sign_extend16(imm);
        if (!psx_memory_write32(memory, address, b)) { cpu->pc = old_pc; cpu->next_pc = old_pc + 4u; return PSX_CPU_STEP_MEMORY_FAULT; }
        break;
    default:
        cpu->pc = old_pc; cpu->next_pc = old_pc + 4u;
        return PSX_CPU_STEP_UNSUPPORTED_INSTRUCTION;
    }

    cpu->gpr[0] = 0u;
    cpu->instructions_retired++;
    return PSX_CPU_STEP_OK;
}
