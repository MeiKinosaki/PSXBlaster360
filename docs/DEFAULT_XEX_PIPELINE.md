# Pipeline de build: default.xex

Projeto: PSXBlaster360 v3.6.15b.

## Estado inicial

O repositório precisa receber o código-fonte completo do projeto antes de uma compilação real. A árvore atual contém apenas a licença e este README; portanto, nenhum workflow pode produzir legitimamente o executável do emulador neste momento.

## Requisitos para XEX

1. Publicar o código-fonte do frontend Xbox 360 e do núcleo PS1, incluindo dependências e instruções de build.
2. Selecionar uma toolchain que produza um executável Xbox 360 compatível com o formato XEX e que possa ser usada legalmente no runner escolhido.
3. Definir o entry point, bibliotecas, importações e configuração de ligação exigidos pelo frontend.
4. Gerar `default.xex` com o linker/empacotador apropriado.
5. Validar o formato, as importações e os símbolos; manter os testes nativos como uma etapa separada.
6. Testar em hardware Xbox 360 RGH antes de declarar compatibilidade.

## Por que não usar somente libxenon?

Uma compilação ELF com libxenon não se torna um XEX por renomeação. O caminho XEX precisa de um ambiente de build e de empacotamento compatíveis com o formato e com o modo de execução pretendido. Não se deve inventar endereços de memória, entry points ou opções do linker.

## CI

O workflow `.github/workflows/xex-toolchain-diagnostic.yml` registra a presença das ferramentas candidatas no runner e publica um relatório de diagnóstico. É intencionalmente diagnóstico: não cria um `default.xex` fictício nem declara o build concluído. Depois que a toolchain real e o código forem incorporados, o workflow pode ser estendido com os comandos documentados dessa toolchain.
