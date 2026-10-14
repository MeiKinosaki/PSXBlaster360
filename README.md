# PSXBlaster360

**Objetivo:** emulador de PlayStation (PS1) para Xbox 360 com RGH.

- Versão de desenvolvimento: **v3.6.17b**
- Artefato final desejado: **`default.xex`**
- Estado: núcleo inicial em C com RAM PS1 e executor parcial MIPS R3000A.
- Ainda não é um emulador completo e não existe `default.xex` validado.

## Implementado

- RAM principal de 2 MiB e janelas de endereçamento RAM/KSEG0/KSEG1.
- Estado/reset inicial da CPU.
- Executor parcial de aritmética/lógica, branches/jumps com delay slot, multiplicação, LW/SW e agora LB/LBU/LH/LHU/SB/SH.
- Testes nativos de memória e execução da CPU no GitHub Actions.

## Próximas etapas

1. Exceções MIPS, overflow e COP0.
2. Instruções restantes, cache/isolation e integração de barramento.
3. GTE, timers, DMA, GPU, SPU e controladores.
4. BIOS/boot path, interface Xbox 360 e toolchain real para `default.xex`.

Os testes nativos no Linux não comprovam compatibilidade com Xbox 360. Renomear um ELF não o converte em XEX. Consulte [o plano de build XEX](docs/DEFAULT_XEX_PIPELINE.md).
