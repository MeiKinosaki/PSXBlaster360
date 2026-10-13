# PSXBlaster360

**Objetivo:** emulador de PlayStation (PS1) para Xbox 360 com RGH.

- Versão do projeto: **v3.6.15b**
- Artefato final desejado: **`default.xex`**
- O formato ELF não é o produto final deste projeto.
- Estado atual do repositório: estrutura inicial; o código-fonte do emulador e uma compilação XEX validada ainda precisam ser publicados.

## Compilação

A criação de `default.xex` exige código-fonte para o alvo Xbox 360 e uma toolchain/empacotador compatível com XEX. Renomear `xenon.elf` para `default.xex` não converte o formato.

Consulte [docs/DEFAULT_XEX_PIPELINE.md](docs/DEFAULT_XEX_PIPELINE.md) para os requisitos e as verificações de CI.

## Status honesto

O workflow de diagnóstico verifica quais ferramentas estão disponíveis no runner do GitHub Actions. Ele não gera um XEX se a toolchain ou o código-fonte estiverem ausentes. Um artefato só será considerado pronto depois de compilar e validar o arquivo `default.xex`.
