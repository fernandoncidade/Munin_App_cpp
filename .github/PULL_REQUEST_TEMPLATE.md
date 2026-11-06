# Modelo de Pull Request / Pull Request Template

<p align="center">
  <b>Selecione o idioma / Select language:</b><br>
  <a href="#ptbr">🇧🇷 Português (BR)</a> | <a href="#enus">🇺🇸 English (US)</a>
</p>

> [!NOTE]
> **Aviso ao Colaborador / Contributor Notice:**  
> 🇧🇷 *Este repositório é um projeto pessoal mantido pelo autor em ritmo próprio de aprendizado. Pull Requests são bem-vindos, mas a análise e incorporação de código ocorrem de forma voluntária e sem compromisso de prazo. Caso não haja manifestação ou merge, sinta-se plenamente livre para manter e utilizar suas melhorias em seu próprio fork sob a licença MIT.*  
> 🇺🇸 *This repository is a personal project maintained by the author at their own learning pace. Pull Requests are welcome, but review and merging are carried out voluntarily without any guaranteed timeframe. If there is no response or merge, please feel free to maintain and use your improvements in your own fork under the MIT license.*

---

## <a id="ptbr"></a>🇧🇷 Português (BR)

<details>
<summary>Clique para expandir / recolher o template em português</summary><br>

## Descrição
Descreva resumidamente o objetivo deste Pull Request, as melhorias implementadas ou os bugs corrigidos no Munin App.

## Tipo de Alteração
- [ ] Correção de bug (alteração sem quebra que corrige um defeito)
- [ ] Nova funcionalidade (novo recurso de visualização, análise ou UI)
- [ ] Melhoria no CLI / Automação (`munin-app-cli`)
- [ ] Otimização de performance (SIMD, paginação de memória virtual ou paralelismo)
- [ ] Melhoria no utilitário de benchmark (`MuninBench`) ou scripts
- [ ] Atualização de documentação / manuais
- [ ] Atualização de traduções (i18n)

## Validação e Testes Realizados
- [ ] Compilado com MinGW Ninja (`cmake --preset host-qt6-mingw-release`)
- [ ] Compilado com MSVC (`cmake --preset host-qt6-msvc-release`)
- [ ] Binário CLI testado (`munin-app-cli.exe --help`)
- [ ] Utilitário de benchmark testado com sucesso (`cmake --preset host-qt6-bench-mingw-release`)
- [ ] Verificação de integridade dos arquivos via `find_VersionEditor --check .` concluída com sucesso
- [ ] Teste manual com arquivos de log grandes (100 MB, 1 GB ou superior)

## Conformidade Arquitetural
- [ ] O código em `source/engine/` (`MuninEngine`) permanece 100% livre de dependências de Qt/GUI.
- [ ] Todas as chamadas de arquivo continuam em modo somente leitura (`GENERIC_READ`).
- [ ] Nenhuma dependência de rede, nuvem ou telemetria foi adicionada (princípio Local-First).
- [ ] Suporte a temas Claro/Escuro do Windows 11 preservado.

## Issues Vinculadas
Corrige #(número da issue)

</details>

---

## <a id="enus"></a>🇺🇸 English (US)

<details>
<summary>Click to expand / collapse the template in English</summary><br>

## Description
Briefly describe the purpose of this Pull Request, improvements made, or bugs fixed in Munin App.

## Type of Change
- [ ] Bug fix (non-breaking change which fixes an issue)
- [ ] New feature (new viewer feature, analysis panel, or UI enhancement)
- [ ] CLI / Automation improvement (`munin-app-cli`)
- [ ] Performance optimization (SIMD, virtual memory paging, or multi-threading)
- [ ] Benchmark utility (`MuninBench`) or tooling improvement
- [ ] Documentation / manual update
- [ ] Translation / localization (i18n) update

## Validation & Testing Performed
- [ ] Compiled with MinGW Ninja (`cmake --preset host-qt6-mingw-release`)
- [ ] Compiled with MSVC (`cmake --preset host-qt6-msvc-release`)
- [ ] CLI binary tested (`munin-app-cli.exe --help`)
- [ ] Benchmark utility tested successfully (`cmake --preset host-qt6-bench-mingw-release`)
- [ ] Project file validation completed via `find_VersionEditor --check .` with all checks passing
- [ ] Manual testing with large log files (100 MB, 1 GB, or larger)

## Architectural Compliance
- [ ] Code in `source/engine/` (`MuninEngine`) remains 100% decoupled from Qt/GUI dependencies.
- [ ] All file mapping operations remain strictly read-only (`GENERIC_READ`).
- [ ] No network, cloud, or telemetry dependencies have been introduced (Local-First principle).
- [ ] Windows 11 Light/Dark theme compatibility is maintained.

## Related Issues
Fixes #(issue number)

</details>
