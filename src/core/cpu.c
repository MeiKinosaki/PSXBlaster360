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
