# PSXBlaster360

**Objetivo:** emulador de PlayStation (PS1) para Xbox 360 com RGH.

- Versão de desenvolvimento: **v3.6.25b**
- Alvo final: **`default.xex`**
- Estado: CPU MIPS parcial, RAM de 2 MiB, interface de carregamento de BIOS de 512 KiB e camada inicial de dispositivos.
- **Ainda não existe `default.xex` validado.**

## Avanços v3.6.25b

- CPU: aritmética/lógica, branches com delay slot, shifts, multiplicação/divisão e loads/stores.
- COP0: suporte inicial a MFC0/MTC0/RFE e exceções selecionadas.
- Memória: RAM little-endian e aliases KSEG0/KSEG1; BIOS opcional de 512 KiB, somente leitura, mapeada em 0x1FC00000 e nos aliases 0x9FC00000/0xBFC00000.
- Barramento: registradores IRQ, três timers, sete canais de registradores DMA e DPCR/DICR; implementada transferência manual limitada de RAM para GPU no canal DMA 2, com atualização de MADR, conclusão e IRQ simplificadas.
- GPU: portas MMIO GP0/GP1 iniciais; GP0 reconhece comandos de ambiente E1-E6 e GP1 modela reset, confirmação de IRQ, display disable, direção DMA e parte do modo de vídeo. Não há FIFO real nem rasterização.
- Sistema: estrutura que reúne CPU, RAM, barramento e relógio simplificado; teste de integração.

## Limitações importantes

A CPU ainda acessa a memória diretamente; os registradores GPU GP0/GP1 já estão expostos no barramento, mas a CPU ainda não os alcança por uma camada MMIO integrada. Timers são simplificados; DMA agora transfere palavras em modo manual RAM→GPU no canal 2, mas os demais modos/canais não transferem dados. A GPU ainda não rasteriza. O carregador de BIOS aceita uma imagem fornecida pelo usuário; ainda não há boot completo validado. CD-ROM, SPU, controladores e interface Xbox 360 seguem pendentes.

Os testes Linux não comprovam compatibilidade com Xbox 360. Renomear um ELF não o converte em XEX; `default.xex` ainda precisa ser gerado e validado com toolchain apropriada.
