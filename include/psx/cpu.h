#ifndef PSX_CPU_H
#define PSX_CPU_H

#include <stdint.h>

typedef struct psx_cpu {
    uint32_t gpr[32];
    uint32_t hi;
    uint32_t lo;
    uint32_t pc;
    uint32_t next_pc;
    uint32_t cop0_status;
    uint32_t cop0_cause;
    uint32_t cop0_epc;
    uint64_t instructions_retired;
} psx_cpu_t;

/* Initializes the architectural state used by the initial CPU core. */
void psx_cpu_reset(psx_cpu_t *cpu);

/* MIPS R3000A sign-extension helpers used by instruction decoding. */
int32_t psx_cpu_sign_extend16(uint16_t immediate);
int32_t psx_cpu_sign_extend8(uint8_t immediate);

#endif
