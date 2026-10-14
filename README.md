# PSXBlaster360

**Objetivo:** emulador de PlayStation (PS1) para Xbox 360 com RGH.

- Versão de desenvolvimento: **v3.6.19b**
- Artefato final desejado: **`default.xex`**
- Estado: núcleo inicial em C com RAM PS1 e executor parcial MIPS R3000A.
- Ainda não é um emulador completo e não existe `default.xex` validado.

## Implementado

- RAM de 2 MiB com janelas de endereçamento RAM/KSEG0/KSEG1.
- Executor parcial MIPS: aritmética/lógica, branches/jumps com delay slot, multiplicação e loads/stores de byte/meia-palavra/palavra.
- Exceções iniciais: overflow, SYSCALL, BREAK, instruções reservadas e erros de alinhamento; registradores COP0 básicos, MFC0/MTC0/RFE.
- Testes nativos automatizados via GitHub Actions.

## Limitações conhecidas

- Não cobre o conjunto completo MIPS R3000A nem todas as exceções e regras de pipeline.
- COP0, interrupções, GTE, timers, DMA, GPU, SPU, controladores e BIOS precisam de mais implementação.
- Ainda não há toolchain XEX integrada nem `default.xex` validado.

Os testes nativos Linux não comprovam compatibilidade com Xbox 360. Renomear um ELF não o converte em XEX. Consulte [o plano de build XEX](docs/DEFAULT_XEX_PIPELINE.md).
