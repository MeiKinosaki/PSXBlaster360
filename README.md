# PSXBlaster360

**Objetivo:** emulador de PlayStation (PS1) para Xbox 360 com RGH.

- Versão de desenvolvimento: **v3.6.20b**
- Alvo final: **`default.xex`**
- Estado: núcleo inicial em C; CPU MIPS R3000A parcial e RAM com espelhamento.
- **Ainda não existe `default.xex` validado.**

## Avanços no núcleo

- CPU: aritmética/lógica, saltos e delay slots, multiplicação, loads/stores de 8/16/32 bits.
- COP0 inicial: MFC0/MTC0/RFE e exceções de overflow, SYSCALL, BREAK, instrução reservada e alinhamento.
- Memória: RAM little-endian, aliases KSEG0/KSEG1 e testes nativos.
- v3.6.20b corrige a preservação de BadVAddr nos erros de alinhamento e acrescenta cobertura para SLTI/SLTIU.

## Próximas frentes

1. Completar CPU/COP0 e exceções precisas.
2. Barramento de memória e mapa de dispositivos.
3. GPU/GP0, framebuffer e rasterização.
4. DMA, timers, SPU, controladores e CD-ROM.
5. BIOS/boot path, interface Xbox 360 e pipeline de build para `default.xex`.

Os testes em Linux verificam apenas os módulos nativos e não comprovam compatibilidade com Xbox 360. Um ELF renomeado não vira XEX.
