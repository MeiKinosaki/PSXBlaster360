# Arquitetura modular e frentes paralelas — PSXBlaster360

Este documento define fronteiras de trabalho para permitir desenvolvimento paralelo sem tratar módulos ainda inexistentes como concluídos.

## Frentes e contratos

| Frente | Responsabilidade | Arquivos atuais / destino | Critério de conclusão |
|---|---|---|---|
| A — Núcleo | CPU MIPS R3000A, instruções, COP0/exceções, RAM e interrupções | `src/core/cpu.c`, `src/core/memory.c`, `src/core/system.c`, `include/psx/cpu.h`, `include/psx/memory.h` | Testes unitários de instruções/exceções e testes integrados CPU–memória aprovados |
| B — Hardware | GPU, DMA, temporizadores, CD-ROM, SPU e periféricos | Inicialmente `src/core/bus.c`, `include/psx/bus.h`; separar GPU/DMA/timers em arquivos próprios em etapas futuras | Testes de registradores, transferência, IRQ e comportamento de dispositivos |
| C — Xbox 360 | Toolchain, bibliotecas, entrada de controle, ELF/XEX e inicialização da plataforma | Criar adaptador de plataforma isolado do núcleo; pipeline XEX precisa de validação real | Compilação reproduzível com toolchain Xenon e `default.xex` gerado/verificado |
| D — Interface e persistência | Biblioteca de jogos, capas, favoritos, configurações e save/load state | Ainda pendente; adicionar em diretório de frontend separado do núcleo | Testes de leitura/escrita, formato versionado e teste de ida-e-volta do estado |
| E — Integração e validação | CI, testes unitários/integrados, falhas, builds e perfilamento | `Makefile`, `.github/workflows/host-tests.yml`, `tests/` | Testes verdes por módulo e integração; artefatos identificados por commit |

## Regras de integração

1. Cada frente altera primeiro os arquivos sob sua responsabilidade e expõe interfaces pequenas nos headers `include/psx/`.
2. Evitar dependências circulares: CPU chama o barramento por sua API; dispositivos não executam instruções da CPU diretamente.
3. Cada mudança funcional inclui teste de regressão. Alterações de documentação não contam como implementação de hardware.
4. Reutilizar código externo somente após verificar licença, origem e compatibilidade com MIPS R3000A/PlayStation 1; código de outros consoles não é presumido compatível.
5. Perfilar antes de otimizar; registrar o comando, compilador, cenário e resultado para que números de desempenho sejam reproduzíveis.
6. Só marcar um marco como concluído quando o critério verificável estiver satisfeito. CI de host não prova compatibilidade com Xbox 360 ou hardware PS1.

## Execução de referência

- `make clean host-test`: compila e executa a suíte completa de host.
- `make build/test_cpu && ./build/test_cpu`: suíte da CPU.
- `make build/test_memory && ./build/test_memory`: suíte da memória.
- `make build/test_bus && ./build/test_bus`: suíte do barramento.
- `make build/test_system && ./build/test_system`: integração do sistema.

## Estado atual

A CPU, RAM e o barramento já têm implementações parciais e testes de host. GPU rasterizada, GTE, CD-ROM, SPU, frontend persistente e execução final no Xbox 360 continuam pendentes. O pipeline de XEX não deve ser considerado validado até que gere e verifique um artefato real.
