# PSXBlaster360

**Objetivo:** emulador de PlayStation (PS1) para Xbox 360 com RGH.

- Versão de desenvolvimento: **v3.6.32b**
- Alvo final: **`default.xex`**
- Estado: CPU MIPS parcial, RAM de 2 MiB, interface de carregamento de BIOS de 512 KiB e camada inicial de dispositivos.
- **Ainda não existe `default.xex` validado.**

## Avanços v3.6.32b

- CPU MIPS: implementadas as instruções LWL/LWR para combinar bytes de palavras alinhadas em registradores, sem exigir que o endereço efetivo esteja alinhado.
- CPU MIPS: implementadas SWL/SWR com leitura-modificação-escrita da palavra alinhada, preservando as lanes de bytes que não pertencem à operação.
- Testes: acrescentados casos de mesclagem de carga e gravação com valores de memória conhecidos; execução da CI da nova revisão ainda precisa terminar para confirmar o resultado.
- Limitação: as operações de merge usam o caminho atual de barramento e não validam ainda todos os casos de borda de MMIO ou timing do processador original.

## Avanços v3.6.31b

- Regressão corrigida: a sequência dos testes de sistema agora restaura explicitamente o PC antes dos testes MMIO de GPU, evitando que os testes de IRQ alterem o endereço de execução esperado.
- IRQ/COP0: testes adicionais verificam que uma fonte fora de I_MASK não levanta IP2 e que uma interrupção reconhecida antes da instrução de delay slot registra EPC do branch e Cause.BD.
- Validação: a suíte de testes de host passou no commit de código/testes anterior à atualização desta versão. A nova revisão de documentação/versão dispara uma nova execução de CI.
- Sem mudança na declaração de compatibilidade: o caminho de IRQ continua sendo um modelo inicial, ainda sem validação em hardware PS1 real ou Xbox 360.

## Avanços v3.6.30b

- CPU/COP0: a CPU integrada consulta IRQ pendentes e mascaradas no barramento, reflete a linha externa no bit IP2 de Cause e entra na exceção de interrupção apenas quando IEc e a máscara correspondente estão habilitados.
- Exceção: a instrução interrompida não é aposentada; o EPC e o vetor normal de exceção são verificados em teste.
- Testes de sistema: cobrem IRQ externa habilitada, fonte fora de I_MASK, interrupções globais desabilitadas e metadados EPC/BD quando a interrupção é reconhecida na janela de delay slot. A suíte de host passou após corrigir a sequência de PC dos testes integrados; ainda faltam validação de prioridades/temporização e comparação com hardware real.

## Avanços v3.6.29b

- Temporizador: no modo modelado de reinício no alvo, os ciclos excedentes agora são preservados como valor residual do contador em vez de serem descartados.
- Teste de regressão: o contador é verificado após 25 ciclos com alvo 10, modo de reinício no alvo habilitado e valor residual esperado 5.

## Avanços v3.6.28b

- CPU/barramento: `LBU` e `LHU` também usam os acessores do barramento no caminho integrado; `LHU` desalinhado gera exceção AdEL e atualiza `BadVAddr`.
- Temporizadores: soma de ciclos usa aritmética de 64 bits para evitar overflow intermediário; leitura do registrador MODE devolve e limpa os flags de alvo/overflow modelados.
- Testes: adicionados casos integrados de `LBU`, `LHU` e `SH`, exceção por desalinhamento, wrap de timer com delta grande, limpeza dos flags de timer e reset de estado GP1/subacessos MMIO.

- CPU/Sistema: busca de instrução e acessos `LB/LBU/LH/LHU/SB/SH/LW/SW` passam pela camada de barramento no modo integrado. Teste de `SW` para GP0 e `SB` para RAM adicionados.
- Barramento: adicionados acessores de 8/16 bits para RAM; subacessos a registradores MMIO de palavra inteira são recusados com segurança.
- Build: alvo de testes da CPU vincula `bus.c`.

- CPU: aritmética/lógica, branches com delay slot, shifts, multiplicação/divisão e loads/stores.
- COP0: suporte inicial a MFC0/MTC0/RFE e exceções selecionadas.
- Memória: RAM little-endian e aliases KSEG0/KSEG1; BIOS opcional de 512 KiB, somente leitura, mapeada em 0x1FC00000 e nos aliases 0x9FC00000/0xBFC00000.
- Barramento: registradores IRQ, três timers, sete canais de registradores DMA e DPCR/DICR; implementada transferência manual limitada de RAM para GPU no canal DMA 2, com atualização de MADR, conclusão e IRQ simplificadas.
- GPU: portas MMIO GP0/GP1 iniciais; GP0 reconhece comandos de ambiente E1-E6 e GP1 modela reset, confirmação de IRQ, display disable, direção DMA e parte do modo de vídeo. Não há FIFO real nem rasterização.
- Sistema: CPU, RAM, barramento e relógio simplificado integrados para buscas de instrução e acessos de palavra; teste de SW até GP0 via MMIO.

## Limitações importantes

A CPU agora encaminha buscas de instrução e acessos LB/LBU/LH/LHU/SB/SH/LW/SW pelo barramento no sistema integrado. Os acessos de byte/meia-palavra à RAM funcionam; registradores MMIO que exigem palavras inteiras recusam acessos menores. A integração MMIO continua parcial. A entrega de interrupção externa para COP0 está modelada de forma inicial em v3.6.30b e ainda precisa de testes mais completos. Timers são simplificados; DMA agora transfere palavras em modo manual RAM→GPU no canal 2, mas os demais modos/canais não transferem dados. A GPU ainda não rasteriza. O carregador de BIOS aceita uma imagem fornecida pelo usuário; ainda não há boot completo validado. CD-ROM, SPU, controladores e interface Xbox 360 seguem pendentes.

Os testes Linux não comprovam compatibilidade com Xbox 360. Renomear um ELF não o converte em XEX; `default.xex` ainda precisa ser gerado e validado com toolchain apropriada.
