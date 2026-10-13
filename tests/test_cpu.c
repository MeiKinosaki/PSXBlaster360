#include "psx/cpu.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    psx_cpu_t cpu;
    unsigned int i;

    memset(&cpu, 0xa5, sizeof(cpu));
    psx_cpu_reset(&cpu);
    for (i = 0; i < 32u; ++i) assert(cpu.gpr[i] == 0u);
    assert(cpu.hi == 0u && cpu.lo == 0u);
    assert(cpu.pc == 0xbfc00000u);
    assert(cpu.next_pc == 0xbfc00004u);
    assert(cpu.cop0_status == 0x10900000u);
    assert(cpu.cop0_cause == 0u && cpu.cop0_epc == 0u);
    assert(cpu.instructions_retired == 0u);

    assert(psx_cpu_sign_extend16(0x7fffu) == 32767);
    assert(psx_cpu_sign_extend16(0x8000u) == -32768);
    assert(psx_cpu_sign_extend16(0xffffu) == -1);
    assert(psx_cpu_sign_extend8(0x7fu) == 127);
    assert(psx_cpu_sign_extend8(0x80u) == -128);
    assert(psx_cpu_sign_extend8(0xffu) == -1);

    psx_cpu_reset(NULL);
    puts("PSXBlaster360 CPU state tests: PASS");
    return 0;
}
