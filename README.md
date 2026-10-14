# PSXBlaster360

**Objetivo:** emulador de PlayStation (PS1) para Xbox 360 com RGH.

- Versão de desenvolvimento: **v3.6.18b**
- Artefato final desejado: **`default.xex`**
- Estado: núcleo inicial em C com RAM PS1 e executor parcial MIPS R3000A.
- Ainda não é um emulador completo e não existe `default.xex` validado.

## Implementado

- RAM de 2 MiB com janelas de endereçamento RAM/KSEG0/KSEG1.
- Executor parcial MIPS: operações aritméticas/lógicas, saltos com delay slot, multiplicação, loads/stores de byte/meia-palavra/palavra.
- Exceções iniciais COP0 para overflow, SYSCALL, BREAK e instruções reservadas; EPC/CAUSE/STATUS e vetor de exceção básico.
- Testes nativos automatizados via GitHub Actions.

## Limitações conhecidas

- O executor ainda não cobre o conjunto completo MIPS R3000A nem todos os casos de exceção/delay slot.
- COP0, interrupções, GTE, timers, DMA, GPU, SPU, controladores e BIOS ainda precisam de implementação.
- Não há ainda toolchain XEX integrada nem `default.xex` validado.

Os testes nativos Linux não comprovam compatibilidade com Xbox 360. Renomear um ELF não o converte em XEX. Consulte [o plano de build XEX](docs/DEFAULT_XEX_PIPELINE.md).
