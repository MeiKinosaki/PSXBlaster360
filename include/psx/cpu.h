#ifndef PSX_CPU_H
#define PSX_CPU_H

#include <stdint.h>
#include "psx/memory.h"

#define PSX_CPU_VERSION "v3.6.20b"

typedef struct psx_cpu {
    uint32_t gpr[32];
    uint32_t hi;
    uint32_t lo;
    uint32_t pc;
    uint32_t next_pc;
    uint32_t cop0_status;
    uint32_t cop0_cause;
    uint32_t cop0_epc;
    uint32_t cop0_bad_vaddr;
    uint64_t instructions_retired;
    uint32_t branch_pc;
    uint8_t branch_delay_pending;
} psx_cpu_t;

typedef enum psx_cpu_step_result {
    PSX_CPU_STEP_OK = 0,
    PSX_CPU_STEP_MEMORY_FAULT,
    PSX_CPU_STEP_UNSUPPORTED_INSTRUCTION,
    PSX_CPU_STEP_EXCEPTION
} psx_cpu_step_result_t;

void psx_cpu_reset(psx_cpu_t *cpu);
int32_t psx_cpu_sign_extend16(uint16_t immediate);
int32_t psx_cpu_sign_extend8(uint8_t immediate);

/* Executes one MIPS R3000A instruction. Branch delay slots are modeled. */
psx_cpu_step_result_t psx_cpu_step(psx_cpu_t *cpu, psx_memory_t *memory);

#endif
