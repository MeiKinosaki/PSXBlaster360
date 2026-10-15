# PSXBlaster360

**Objetivo:** emulador de PlayStation (PS1) para Xbox 360 com RGH.

- Versão de desenvolvimento: **v3.6.21b**
- Alvo final: **`default.xex`**
- Estado: núcleo em C com CPU MIPS R3000A parcial, RAM e primeira camada de barramento/dispositivos.
- **Ainda não existe `default.xex` validado.**

## Avanços v3.6.21b

- CPU: operações aritméticas/lógicas, branches com delay slot, shifts, multiplicação/divisão e loads/stores.
- COP0: suporte inicial a MFC0/MTC0/RFE e exceções selecionadas.
- Memória: RAM de 2 MiB, aliases KSEG0/KSEG1 e acessos little-endian.
- Barramento inicial: registradores de IRQ, três timers, registradores de sete canais DMA e DPCR/DICR.
- GPU: reconhecimento inicial dos comandos GP0 de ambiente E1-E6.
- Testes nativos para RAM, CPU e novos registradores de barramento/dispositivos.

## Limitações importantes

A camada de barramento ainda não está ligada à execução da CPU como MMIO completo. Timers têm temporização simplificada; DMA ainda não executa transferências; GPU ainda não rasteriza primitivas. COP0 e CPU permanecem parciais. BIOS, CD-ROM, SPU, controladores e interface Xbox 360 ainda precisam de implementação.

Os testes Linux não comprovam compatibilidade com Xbox 360. Renomear um ELF não o converte em XEX; `default.xex` ainda precisa ser gerado e validado com toolchain apropriada.
