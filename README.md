# PSXBlaster360

**Objetivo:** emulador de PlayStation (PS1) para Xbox 360 com RGH.

- Versão de desenvolvimento: **v3.6.22b**
- Alvo final: **`default.xex`**
- Estado: CPU MIPS parcial, RAM de 2 MiB, interface de carregamento de BIOS de 512 KiB e camada inicial de dispositivos.
- **Ainda não existe `default.xex` validado.**

## Avanços v3.6.22b

- CPU: aritmética/lógica, branches com delay slot, shifts, multiplicação/divisão e loads/stores.
- COP0: suporte inicial a MFC0/MTC0/RFE e exceções selecionadas.
- Memória: RAM little-endian e aliases KSEG0/KSEG1; BIOS opcional de 512 KiB, somente leitura, mapeada em 0x1FC00000 e nos aliases 0x9FC00000/0xBFC00000.
- Barramento: registradores IRQ, três timers, sete canais de registradores DMA e DPCR/DICR.
- GPU: reconhecimento inicial dos comandos de ambiente GP0 E1-E6 (sem rasterização).
- Sistema: estrutura que reúne CPU, RAM, barramento e relógio simplificado; teste de integração.

## Limitações importantes

A CPU ainda acessa RAM diretamente; o barramento não está integrado como MMIO completo. Timers são simplificados, DMA não transfere dados e GPU não rasteriza. O carregador de BIOS aceita uma imagem fornecida pelo usuário; ainda não há boot completo validado. CD-ROM, SPU, controladores e interface Xbox 360 seguem pendentes.

Os testes Linux não comprovam compatibilidade com Xbox 360. Renomear um ELF não o converte em XEX; `default.xex` ainda precisa ser gerado e validado com toolchain apropriada.
