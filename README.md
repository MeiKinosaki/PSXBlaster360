# PSXBlaster360

**Objetivo:** emulador de PlayStation (PS1) para Xbox 360 com RGH.

- Versão de desenvolvimento: **v3.6.16b**
- Artefato final desejado: **`default.xex`**
- Estado: núcleo inicial em C com RAM PS1, estado da CPU MIPS R3000A e executor parcial de instruções.
- Ainda não é um emulador completo e não existe `default.xex` validado.

## Implementado

- RAM principal de 2 MiB com validação de janelas físicas/KSEG0/KSEG1.
- Testes nativos de memória.
- Estado/reset inicial da CPU.
- Executor parcial: operações aritméticas e lógicas, branches/jumps com delay slot, multiplicação, LW/SW.
- Testes automatizados em GitHub Actions via `make host-test`.

## Próximas etapas

1. Exceções MIPS e COP0 completo.
2. GTE, timers, DMA, GPU, SPU e controladores.
3. BIOS/boot path e interface específica para Xbox 360.
4. Toolchain Xbox 360 e pipeline real para `default.xex`.

A compilação nativa testa os módulos no runner Linux; não comprova compatibilidade com o Xbox 360. Renomear um ELF não o converte em XEX. Consulte [o plano de build XEX](docs/DEFAULT_XEX_PIPELINE.md).
