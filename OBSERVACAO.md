<!-- Multilanguage OBSERVACAO.md for Munin_App_cpp -->

<p align="center">
  <b>Selecione o idioma / Select language:</b><br>
  <a href="#ptbr">🇧🇷 Português (BR)</a> |
  <a href="#enus">🇺🇸 English (US)</a> |
  <a href="#tree">🌳 Árvore de Arquivos / File Tree</a>
</p>

---

<h2 id="ptbr">🇧🇷 Português (BR)</h2>

<details>
<summary>Clique para expandir / recolher a Observação Técnica em português</summary>

# OBSERVAÇÃO TÉCNICA - Estado atual do projeto

Versão documentada: `2026.11.6.0`  
Data do snapshot técnico: `6 de novembro de 2026`  
Repositório: `Munin_App_cpp`  
Autor identificado no projeto: `Fernando Nillsson Cidade`  
Licença do repositório: `GNU Affero General Public License v3 (AGPLv3)`  

---

## 1. Escopo deste Documento

Este documento registra o estado técnico real do projeto **Munin App — Leitor de Logs Massivos** após a conclusão das implementações, testes e validações consolidadas até a versão oficial **`2026.11.6.0`**.

O objetivo é documentar de forma detalhada:
- A arquitetura da engine de processamento em **C++20** de baixo nível com APIs nativas Win32 e aceleração vetorial SIMD;
- A camada de apresentação em **Qt 6.11.1** com componentes virtualizados, minimapa de densidade, inspetor JSON e histograma temporal;
- Os utilitários de linha de comando (`munin-app-cli` e `MuninBench`);
- Os fluxos de build, presets e deployment automatizado via CMake e Ninja;
- A organização de diretórios do repositório `Munin_App_cpp`;
- Os padrões de persistência local em `%LOCALAPPDATA%/Munin`;
- As ferramentas de desenvolvimento e mocks (`find_VersionEditor`, compiladores de tradução);
- As regras de empacotamento com Inno Setup e manifesto MSIX para distribuição independente em MinGW e MSVC.

---

## 2. Linha Técnica e Destaques da Versão 2026.11.6.0

A versão 2026.11.6.0 marca o lançamento oficial do Munin App como leitor de logs massivos para Windows 11, entregando:

| Componente | Tipo | Estado Operacional |
| :--- | :--- | :--- |
| **MuninEngine** | Biblioteca Estática C++20 | Motor de baixo nível independente de GUI, com mapeamento Win32, vetorização SIMD (AVX2/SSE4.2), Live Tail e busca paralela. |
| **Munin** | Executável Gráfico Qt6 | Aplicação desktop principal com virtualização de tabela, minimapa, linha do tempo Time-Slice, inspetor JSON e temas adaptativos. |
| **munin-app-cli** | Executável de Linha de Comando C++20 | Utilitário standalone de linha de comando para automação, busca paralela, extração de métricas de log e streaming live tail sem inicialização gráfica. |
| **MuninBench** | Executável de Teste C++20 | Utilitário de linha de comando para aferição de taxa de transferência (GB/s), indexação SIMD e busca multi-core. |
| **find_VersionEditor** | Utilitário Mock (GUI / CLI) | Ferramenta com interface gráfica e suporte a CLI (`--check`, `--update`) para validação e atualização síncrona de versões e datas. |
| **CompileTranslations** | Utilitários Mock (GUI / CLI) | Compiladores de arquivos de tradução Qt (`.ts` para `.qm`) em modo gráfico e terminal. |

### Principais Marcos Consolidados:
1. **Engine Win32 Zero-Copy de Alta Performance**:
   - Mapeamento direto de arquivos em memória virtual (`MemoryMappedFile`) com chamadas nativas `CreateFileMappingW` e `MapViewOfFile`, dispensando alocação prévia de buffers e permitindo abrir arquivos de dezenas de gigabytes instantaneamente;
   - Suporte a compartilhamento concorrente não-bloqueante (`FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE`).
2. **Indexação Vetorial por Instruções SIMD**:
   - Escaneamento de quebras de linha (`\n`) acelerado por registradores de 256 bits (`_mm256_movemask_epi8`) em hardware AVX2 e 128 bits (`_mm_movemask_epi8`) em SSE4.2, alcançando taxas de transferência superiores a 3 GB/s;
   - Fallback escalar inteligente e robusto para processadores legados.
3. **Monitoramento Live Tail em Tempo Real**:
   - Acompanhamento assíncrono de arquivos em escrita concorrente utilizando notificações de kernel do Windows via `ReadDirectoryChangesW`, sem consumo de CPU por polling;
   - Rolagem automática inteligente e expansão incremental de índices sem reindexação do arquivo.
4. **Busca Paralela Multi-Core e Filtragem JIT**:
   - Divisão de tarefas por blocos de 64 KB entre todas as threads de hardware lógicas disponíveis;
   - Suporte a busca literal, diferenciação de maiúsculas/minúsculas, palavras inteiras, Regex compilado e modo de filtragem de linhas.
5. **Timeline de Distribuição Temporal (Time-Slice)**:
   - Extração automática de timestamps ISO-8601 e exibição de histograma interativo para identificação imediata de surtos de erro e picos de tráfego.
6. **Inspetor JSON Estruturado e Navegável**:
   - Painel retrátil que formata payloads complexos em árvore hierárquica navegável com realce sintático e cópia com um clique.
7. **Barra de Rolagem com Minimapa de Densidade**:
   - Renderização visual proporcional da densidade de eventos ao longo do arquivo, com marcadores cromáticos em vermelho (erros), amarelo (avisos) e azul/ciano (resultados de busca).
8. **Adaptação Automática ao Tema do Windows 11**:
   - Detecção contínua de eventos de alternância de tema Claro/Escuro do sistema operacional, recalculando a paleta de cores de toda a UI dinamicamente;
   - Integração com a API Win32 Desktop Window Manager (DWM) para barras de título escuras imersivas e efeito Mica.
9. **Internacionalização Dinâmica em Tempo de Execução**:
   - Suporte completo a Português do Brasil (`pt_BR`) e Inglês (`en_US`), com retradução instantânea de menus, botões, status bar, tooltips e diálogos sem necessidade de reiniciar.
10. **Hierarquização de Janelas Modeless (Z-Order Independente)**:
    - Diálogos Sobre e Manual de Instruções estruturados como janelas de nível superior independentes, permitindo foco e alternância de sobreposição livres na interface principal e compatibilidade plena com múltiplos monitores.
11. **Utilitário de Linha de Comando Autônomo e Integração Terminal**:
    - Binário headless `munin-app-cli.exe` construído para ambos os compiladores (MinGW GCC 13.1.0 e MSVC 2022 v143);
    - Registro de `AppExecutionAlias` no pacote MSIX (`repo/AppxManifest.xml`) e scripts Inno Setup (`repo/Munin.iss`, `Munin_MinGW.iss`, `Munin_MSVC.iss`) permitindo chamada direta de qualquer console;
    - Operações de extração de métricas (`info`), busca paralela com filtros avançados e exportação JSON (`search`), streaming ao vivo com detecção de rotação de logs (`tail`) e benchmarks (`bench`).
12. **Sincronização Abrangente de Metadados (28 Arquivos Monitorados)**:
    - O utilitário `find_VersionEditor` rastreia 28 arquivos críticos (código C++, licenças EULA, contratos comerciais CLC, avisos NOTICE, políticas de privacidade, documentação completa em `docs/`, `README.md`, `MANUAL.md`, `OBSERVACAO.md` e instaladores Inno Setup), garantindo conformidade rigorosa.

---

## 3. Arquitetura de Alto Nível

O projeto adota uma arquitetura em camadas rigorosamente desacopladas:

```text
+------------------------------------------------------------------------------------+
|                                    Munin (GUI)                                     |
|  MainWindow | LogView | MinimapScrollBar | SearchBar | TimeSlice | JsonInspector   |
+------------------------------------------------------------------------------------+
|                         Camada de Apresentação e Qt 6                              |
|   LogModel (QAbstractTableModel) | ThemeManager | Win32DwmHelper | GerenciadorTraducao|
+------------------------------------------------------------------------------------+
|                           MuninEngine (Engine Nativa C++20)                        |
|  MemoryMappedFile | SimdUtils | ChunkIndex | LiveTailWatcher | ParallelSearch     |
+------------------------------------------------------------------------------------+
|                              Kernel do Windows NT                                  |
|   Win32 CreateFileMappingW / MapViewOfFile | ReadDirectoryChangesW | AVX2/SSE4.2   |
+------------------------------------------------------------------------------------+
```

### 3.1. Engine de Baixo Nível (`source/engine/`)

- **`MemoryMappedFile.hpp/.cpp`**:
  - Encapsula manipuladores Win32 (`HANDLE`) para arquivo e mapeamento de seção.
  - Oferece acesso via ponteiro bruto constante (`const char* data() const`) e tamanho de arquivo de 64 bits (`uint64_t size() const`).
  - Garante liberação atômica e segura dos recursos no destrutor (`UnmapViewOfFile`, `CloseHandle`).
- **`SimdUtils.hpp/.cpp`**:
  - Implementa funções vetoriais de escaneamento de memória:
    - `count_newlines_avx2(const char* data, size_t size)`: processa 32 bytes por ciclo;
    - `count_newlines_sse42(const char* data, size_t size)`: processa 16 bytes por ciclo;
    - `count_newlines_scalar(const char* data, size_t size)`: fallback para alinhamento final e máquinas sem suporte vetorial;
    - `find_newlines_simd(...)`: armazena os deslocamentos relativos de cada quebra de linha.
- **`ChunkIndex.hpp/.cpp`**:
  - Segmenta o arquivo mapeado em blocos lógicos (*chunks*) de 64 KB (`65536` bytes).
  - Mantém tabelas com os índices das primeiras linhas de cada bloco para permitir busca binária acelerada em arquivos com mais de 50 milhões de linhas.
  - Suporta expansão incremental no final do arquivo sem invalidar blocos anteriores.
- **`LiveTailWatcher.hpp/.cpp`**:
  - Cria uma thread dedicada de monitoramento que registra o diretório do arquivo ativo em `ReadDirectoryChangesW`.
  - Ao detectar incremento de tamanho no arquivo, notifica a camada gráfica através de fila thread-safe para atualização imediata dos offsets e rolagem suave.
- **`ParallelSearch.hpp/.cpp`**:
  - Recebe critérios de busca estruturados (`SearchOptions`: padrão textual/regex, case-sensitive, whole-word, modo de filtro).
  - Subdivide a quantidade total de blocos pelo número de núcleos lógicos disponíveis (`std::thread::hardware_concurrency()`).
  - Executa a busca em paralelo sem contenção de mutexes e agrega as ocorrências (`SearchMatch`) de forma ordenada por número de linha.
- **`TimeSliceScanner.hpp/.cpp`**:
  - Extrai carimbos de tempo em formatos padrão (ISO 8601, logs de Apache, Nginx, syslog, etc.).
  - Agrupa as ocorrências em baldes temporais (*buckets*) para alimentação do histograma gráfico.
- **`TestEngine.cpp` (`MuninBench`)**:
  - Suíte de benchmarking que avalia a vazão de I/O de disco, taxa de transferência SIMD e escalabilidade multi-thread.

---

### 3.2. Utilitário de Linha de Comando (`source/cli/`)

- **`main_cli.cpp` (`munin-app-cli`)**:
  - Utilitário autônomo de linha de comando compilado nativamente sem carregar os módulos de GUI do Qt.
  - Suporta os subcomandos `search`, `info`, `tail`, `bench`, além de `--version` e `--help`.
  - Habilita integração headless com pipelines de CI/CD, terminais remotos do Windows e scripts PowerShell.

---

### 3.3. Camada Gráfica e Modelos (`source/model/` e `source/ui/`)

- **`LogModel.hpp/.cpp`**:
  - Herda de `QAbstractTableModel`. Não armazena cópias das linhas na memória heap do Qt;
  - O método `data(const QModelIndex&, int role)` lê as fatias de texto diretamente da memória mapeada usando os offsets do `ChunkIndex`, decodificando e renderizando sob demanda;
  - Suporta modo filtrado onde apenas as linhas correspondentes à pesquisa ativa são expostas às views.
- **`MainWindow.hpp/.cpp`**:
  - Centraliza a barra de menus `StickyMenu`, a barra de ferramentas superior com botões de atalho, os painéis divisores (`QSplitter`) e a barra de status;
  - Responde a eventos nativos de sistema do Windows (`WM_SETTINGCHANGE`, `nativeEvent`) para sincronização em tempo real de tema e cores.
- **`LogView.hpp/.cpp`**:
  - Customização de `QTableView` projetada para fluidez extrema, scroll suave por pixel e seleção de texto ágil;
  - Integração direta com a barra de rolagem customizada `MinimapScrollBar`.
- **`MinimapScrollBar.hpp/.cpp`**:
  - Renderiza um minimapa analítico na calha vertical da rolagem, mapeando proporções lineares de todo o arquivo e desenhando marcadores visuais coloridos para erros, avisos e resultados de busca.
- **`SearchBarWidget.hpp/.cpp`**:
  - Barra de pesquisa integrada com suporte a regex, diferenciação de maiúsculas/minúsculas, seleção de palavra inteira, navegação entre ocorrências e ativação do modo de filtragem.
- **`TimeSliceWidget.hpp/.cpp`**:
  - Widget com gráfico interativo de barras temporais que permite inspecionar a distribuição de registros ao longo do tempo e filtrar intervalos específicos com um clique.
- **`JsonInspectorWidget.hpp/.cpp`**:
  - Inspetor detalhado que detecta e formata payloads JSON presentes nas linhas selecionadas, exibindo propriedades em árvore expansível e oferecendo cópia rápida.
- **`LogGeneratorDialog.hpp/.cpp`**:
  - Diálogo de geração de arquivos de log sintéticos de 100 MB, 500 MB e 1 GB para aferição e testes de estresse.
- **`ThemeManager.hpp/.cpp` e `Win32DwmHelper.hpp/.cpp`**:
  - Gerenciador centralizado de paletas claras e escuras do Qt;
  - Utiliza APIs do DWM do Windows para aplicação de barra de título escura/clara imersiva (`DwmSetWindowAttribute`).
- **`GerenciadorDialogos.hpp/.cpp`**:
  - Gerencia o ciclo de vida e a sobreposição (Z-Order) dos diálogos `SobreDialog` e `ManualDialog`, garantindo que ambos operem como janelas modeless desacopladas.

---

### 3.4. Internacionalização (`source/language/`)

- **`GerenciadorTraducao.hpp/.cpp`**:
  - Gerencia a instalação e remoção de tradutores `QTranslator` em tempo de execução.
  - Carrega dinamicamente os catálogos compilados:
    - `source/language/translations/munin_pt_BR.qm`
    - `source/language/translations/munin_en_US.qm`
  - Propaga eventos `QEvent::LanguageChange` para que todos os widgets da UI, tooltips, menus e diálogos reescrevam seus textos instantaneamente.

---

### 3.5. Mocks e Ferramentas Auxiliares (`mocks/`)

- **`find_VersionEditor.cpp/.hpp`**:
  - Utilitário gráfico e de linha de comando para validação e atualização síncrona de versão e data em todos os 28 arquivos críticos do projeto:
    - `source/assets/ABOUT/ABOUT_pt_BR.txt` e `ABOUT_en_US.txt`
    - `source/assets/CLC/CLC_pt_BR.txt` e `CLC_en_US.txt`
    - `source/assets/EULA/EULA_pt_BR.txt` e `EULA_en_US.txt`
    - `source/assets/NOTICES/NOTICE_pt_BR.txt` e `NOTICE_en_US.txt`
    - `source/assets/PRIVACY_POLICY/Privacy_Policy_pt_BR.txt` e `Privacy_Policy_en_US.txt`
    - `main.cpp`, `source/main.cpp`, `source/cli/main_cli.cpp`
    - `source/ui/SobreDialog.cpp`, `source/ui/GerenciadorDialogos.cpp`
    - `README.md`, `MANUAL.md`, `OBSERVACAO.md`
    - `docs/architecture.md`, `docs/cli-reference.md`, `docs/compliance.md`, `docs/licensing.md`, `docs/security-model.md`, `docs/user-guide.md`, `docs/release-v2026.11.6.0.md`
    - `repo/Munin.iss`, `repo/Munin_MinGW.iss` e `repo/Munin_MSVC.iss`
  - Suporte a verificação de integridade via CLI: `find_VersionEditor.exe --check`
  - Suporte a atualização síncrona via CLI: `find_VersionEditor.exe --update [pasta] [versão]`
- **`CompileTranslationsGUI.cpp` e `CompileTranslationsTerminal.cpp`**:
  - Utilitários para compilação automatizada dos arquivos de tradução `.ts` para binários `.qm`.

---

## 4. Persistência e Armazenamento Local

O Munin App respeita as diretrizes de armazenamento do Windows para aplicativos corporativos e desktop:

- **Diretório Raiz de Configurações**:
  - `%LOCALAPPDATA%/Munin/`
- **Itens Persistidos**:
  - `config.ini` ou configurações via `QSettings`:
    - Idioma ativo selecionado pelo usuário (`pt_BR` ou `en_US`);
    - Geometria e estado da janela principal (tamanho, posição, estado maximizado, divisores de splitter);
    - Preferências visuais da barra de ferramentas e painéis retráteis;
- **Arquivos Temporários**:
  - O aplicativo opera com arquivos mapeados diretamente de seus locais de origem, sem gerar arquivos temporários redundantes em disco.

---

## 5. Build CMake, Presets e Targets

O projeto é configurado com CMake 3.21+ e arquitetura baseada em presets declarativos em `CMakePresets.json`:

### 5.1. Presets de Configuração e Build

| Preset | Compilador | Tipo | Descrição |
| :--- | :--- | :--- | :--- |
| `host-qt6-mingw-release` | MinGW GCC 13.1.0+ | Release | Compila o Munin App com toolchain MinGW e runtime Qt 6.11.1 (Ninja). |
| `host-qt6-msvc-release` | MSVC 2022 v143+ | Release | Compila o Munin App com toolchain MSVC e runtime Qt 6.11.1 (Ninja). |
| `host-qt6-bench-mingw-release` | MinGW GCC 13.1.0+ | Release | Compila o benchmark MuninBench via MinGW. |
| `host-qt6-bench-msvc-release` | MSVC 2022 v143+ | Release | Compila o benchmark MuninBench via MSVC. |
| `host-qt6-mocks-mingw-release` | MinGW GCC 13.1.0+ | Release | Compila os utilitários de suporte e mocks (find_VersionEditor) via MinGW. |
| `host-qt6-mocks-msvc-release` | MSVC 2022 v143+ | Release | Compila os utilitários de suporte e mocks (find_VersionEditor) via MSVC. |

### 5.2. Targets de Compilação Principais

- **`MuninEngine`**: Biblioteca estática que compila os componentes de motor de baixo nível C++20.
- **`Munin`**: Executável gráfico principal integrado com Qt6.
- **`munin-app-cli`**: Executável de linha de comando autônomo compilado a partir de `source/cli/main_cli.cpp`.
- **`MuninBench`**: Executável de linha de comando para benchmarks de I/O e SIMD.
- **`MuninDeploy`**: Alvo customizado que automatiza a execução do `windeployqt.exe`, cópia de plugins nativos (estilos, plataformas, svg), catálogos de tradução `.qm`, assets e sincronização da distribuição limpa externa.
- **`find_VersionEditor` / `find_VersionEditorDeploy`**: Alvos dos utilitários de desenvolvimento.

### 5.3. Distribuições Limpas

Após a compilação com o target `MuninDeploy`, as distribuições limpas e prontas para uso são geradas em:
- `D:\MISCELANEAS\VSCode\Munin\Munin_mingw\`
- `D:\MISCELANEAS\VSCode\Munin\Munin_msvc\`

Ambas contêm `Munin.exe`, `munin-app-cli.exe`, `MuninBench.exe` e todos os runtimes e assets necessários.

---

## 6. Empacotamento com Inno Setup e MSIX

Para criação dos instaladores profissionais do Windows, o projeto dispõe de scripts na pasta `repo/`:

- **`repo/Munin.iss`**: Script mestre unificado que compila tanto a versão MinGW quanto a versão MSVC conforme parâmetro `/DBuildFlavor=mingw` ou `/DBuildFlavor=msvc`.
- **`repo/Munin_MinGW.iss`**: Script focado no empacotamento da distribuição MinGW (`Munin_mingw_v2026.11.6.0.exe`).
- **`repo/Munin_MSVC.iss`**: Script focado no empacotamento da distribuição MSVC (`Munin_msvc_v2026.11.6.0.exe`).
- **`repo/AppxManifest.xml`**: Manifesto para empacotamento MSIX da Microsoft Store com `AppExecutionAlias` para `munin` e `munin-app-cli`.
- **Diretório de Saída dos Instaladores**:
  - `D:\MISCELANEAS\VSCode\Munin\InnoSetupOutput\`

---

## 7. Conclusão

O projeto **Munin_App_cpp** encontra-se em estado maduro, estável e plenamente operacional na versão **`2026.11.6.0`**, com compilação e testes validados nos ambientes **MinGW** e **MSVC**, suporte completo a interface desktop e utilitário de linha de comando headless, verificação de integridade automatizada em 100% dos arquivos monitorados via `find_VersionEditor --check` e geração automatizada de distribuições e instaladores limpos.

</details>

---

<h2 id="enus">🇺🇸 English (US)</h2>

<details>
<summary>Click to expand / collapse Technical Note in English</summary>

# TECHNICAL NOTE - Current Project State

Documented version: `2026.11.6.0`  
Technical snapshot date: `November 6, 2026`  
Repository: `Munin_App_cpp`  
Author identified in the project: `Fernando Nillsson Cidade`  
Repository License: `GNU Affero General Public License v3 (AGPLv3)`  

---

## 1. Scope of this Document

This document records the actual technical state of the **Munin App — Massive Log Viewer** project upon completion of implementations, tests, and consolidated validations up to the official release **`2026.10.18.0`**.

The objective is to thoroughly document:
- The low-level **C++20** processing engine architecture featuring native Win32 APIs and SIMD vector acceleration;
- The **Qt 6.11.1** presentation layer with virtualized components, density minimap, JSON inspector, and temporal histogram;
- The command-line utilities (`munin-app-cli` and `MuninBench`);
- Automated build, presets, and deployment workflows via CMake and Ninja;
- Directory structure and repository organization of `Munin_App_cpp`;
- Local persistence standards in `%LOCALAPPDATA%/Munin`;
- Development tools and mocks (`find_VersionEditor`, translation compilers);
- Packaging configurations using Inno Setup and MSIX manifests for independent MinGW and MSVC distributions.

---

## 2. Technical Line and Highlights of Version 2026.11.6.0

Release 2026.10.18.0 marks the official launch of Munin App as a massive log reader for Windows 11, delivering:

| Component | Type | Operational Status |
| :--- | :--- | :--- |
| **MuninEngine** | C++20 Static Library | Low-level GUI-independent engine featuring Win32 memory mapping, SIMD vectorization (AVX2/SSE4.2), Live Tail, and parallel search. |
| **Munin** | Qt6 Graphical Executable | Primary desktop application with virtualized table, minimap, Time-Slice timeline, JSON inspector, and adaptive themes. |
| **munin-app-cli** | C++20 CLI Executable | Standalone command-line utility for automation, parallel search, log metadata extraction, and streaming live tail without GUI initialization. |
| **MuninBench** | C++20 Test Executable | Command-line benchmark utility measuring throughput (GB/s), SIMD indexing, and multi-core search scaling. |
| **find_VersionEditor** | Mock Utility (GUI / CLI) | Tool with graphical UI and CLI support (`--check`, `--update`) for synchronous version and date validation and updating. |
| **CompileTranslations** | Mock Utilities (GUI / CLI) | Compilers for Qt translation catalogs (`.ts` to `.qm`) in graphical and console modes. |

### Major Consolidated Milestones:
1. **High-Performance Zero-Copy Win32 Engine**:
   - Direct virtual memory file mapping (`MemoryMappedFile`) with native `CreateFileMappingW` and `MapViewOfFile` calls, bypassing buffer pre-allocations and opening multi-gigabyte files instantaneously;
   - Non-blocking concurrent sharing support (`FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE`).
2. **SIMD Vectorized Indexing**:
   - Newline delimiter scanning (`\n`) accelerated by 256-bit registers (`_mm256_movemask_epi8`) on AVX2 hardware and 128-bit (`_mm_movemask_epi8`) on SSE4.2, achieving throughput exceeding 3 GB/s;
   - Robust and intelligent scalar fallback for legacy processors.
3. **Real-Time Live Tail Monitoring**:
   - Asynchronous tracking of actively written files via Windows kernel notifications using `ReadDirectoryChangesW`, consuming zero CPU cycles on polling;
   - Intelligent auto-scroll and incremental index growth without re-indexing the entire document.
4. **Multi-Core Parallel Search and JIT Filtering**:
   - Workload partitioning into 64 KB chunks across all available logical CPU hardware threads;
   - Support for literal string search, case sensitivity, whole words, compiled Regex, and matched-lines filter mode.
5. **Time Distribution Timeline (Time-Slice)**:
   - Automatic ISO-8601 timestamp parsing and interactive histogram visualization for immediate triage of error bursts and traffic surges.
6. **Structured and Navigable JSON Inspector**:
   - Collapsible panel parsing complex payloads into a navigable hierarchical tree with syntax highlighting and one-click copy.
7. **Minimap Density Scrollbar**:
   - Proportional visual heatmap of event density throughout the file, featuring chromatic markers in red (errors), yellow (warnings), and cyan/blue (search matches).
8. **Automatic Adaptation to Windows 11 Theme**:
   - Continuous detection of OS Dark/Light theme switching events, dynamically recalculating the palette across all UI widgets;
   - Integration with the Win32 Desktop Window Manager (DWM) API for immersive dark title bars and Mica styling.
9. **Dynamic Runtime Internationalization**:
   - Comprehensive support for Brazilian Portuguese (`pt_BR`) and US English (`en_US`), with instantaneous retranslation of menus, buttons, status bar, tooltips, and dialogs without restart.
10. **Modeless Window Hierarchy (Independent Z-Order)**:
    - About and Instruction Manual dialogs built as independent top-level windows, granting unrestricted focus and overlapping navigation across the main interface and multi-monitor setups.
11. **Standalone Command-Line Utility and Terminal Integration**:
    - Headless binary `munin-app-cli.exe` compiled for both toolchains (MinGW GCC 13.1.0 and MSVC 2022 v143);
    - Registered `AppExecutionAlias` in the MSIX manifest (`repo/AppxManifest.xml`) and Inno Setup scripts (`repo/Munin.iss`, `Munin_MinGW.iss`, `Munin_MSVC.iss`), enabling direct console invocation;
    - High-speed operations: metric inspection (`info`), multi-core search with JSON export (`search`), live streaming with log rotation handling (`tail`), and benchmarks (`bench`).
12. **Comprehensive Metadata Synchronization (28 Monitored Files)**:
    - Expanded `find_VersionEditor` suite monitoring 28 critical project files (C++ codebases, EULA licenses, commercial contracts CLC, NOTICE files, privacy policies, full `docs/` technical suite, `README.md`, `MANUAL.md`, `OBSERVACAO.md`, and Inno Setup installers) ensuring absolute cross-file consistency.

---

## 3. High-Level Architecture

The project enforces a strictly decoupled layered architecture:

```text
+------------------------------------------------------------------------------------+
|                                    Munin (GUI)                                     |
|  MainWindow | LogView | MinimapScrollBar | SearchBar | TimeSlice | JsonInspector   |
+------------------------------------------------------------------------------------+
|                         Presentation & Qt 6 Layer                                  |
|   LogModel (QAbstractTableModel) | ThemeManager | Win32DwmHelper | GerenciadorTraducao|
+------------------------------------------------------------------------------------+
|                           MuninEngine (Native C++20 Engine)                        |
|  MemoryMappedFile | SimdUtils | ChunkIndex | LiveTailWatcher | ParallelSearch     |
+------------------------------------------------------------------------------------+
|                              Windows NT Kernel                                     |
|   Win32 CreateFileMappingW / MapViewOfFile | ReadDirectoryChangesW | AVX2/SSE4.2   |
+------------------------------------------------------------------------------------+
```

### 3.1. Low-Level Engine (`source/engine/`)

- **`MemoryMappedFile.hpp/.cpp`**:
  - Encapsulates Win32 system handles (`HANDLE`) for file and section mapping.
  - Exposes raw constant memory pointers (`const char* data() const`) and 64-bit file size (`uint64_t size() const`).
  - Ensures atomic and safe resource cleanup in destructor (`UnmapViewOfFile`, `CloseHandle`).
- **`SimdUtils.hpp/.cpp`**:
  - Implements memory-scanning vector routines:
    - `count_newlines_avx2(const char* data, size_t size)`: processes 32 bytes per cycle;
    - `count_newlines_sse42(const char* data, size_t size)`: processes 16 bytes per cycle;
    - `count_newlines_scalar(const char* data, size_t size)`: fallback for trailing alignment and non-vector CPUs;
    - `find_newlines_simd(...)`: records relative offsets for each newline delimiter.
- **`ChunkIndex.hpp/.cpp`**:
  - Partitions mapped memory into 64 KB (`65536` bytes) sparse chunks.
  - Maintains lookup tables of first-line indexes per chunk, enabling accelerated binary seeking across 50M+ rows.
  - Supports incremental append growth at file EOF without invalidating previous chunks.
- **`LiveTailWatcher.hpp/.cpp`**:
  - Runs a dedicated worker thread monitoring the parent folder with `ReadDirectoryChangesW`.
  - Upon detecting file growth, dispatches notifications to the UI layer via a thread-safe queue for smooth auto-scrolling.
- **`ParallelSearch.hpp/.cpp`**:
  - Consumes structured query parameters (`SearchOptions`: string/regex pattern, case-matching, whole word, filter mode).
  - Divides chunks evenly across logical CPU cores (`std::thread::hardware_concurrency()`).
  - Executes search in parallel with zero mutex contention, aggregating matches (`SearchMatch`) sorted by line number.
- **`TimeSliceScanner.hpp/.cpp`**:
  - Extracts standard timestamps (ISO-8601, Apache, Nginx, syslog, etc.).
  - Bins log records into temporal buckets feeding the graphical histogram.
- **`TestEngine.cpp` (`MuninBench`)**:
  - Benchmarking suite quantifying disk I/O throughput, SIMD scanning speed, and multi-thread scalability.

---

### 3.2. Command-Line Utility (`source/cli/`)

- **`main_cli.cpp` (`munin-app-cli`)**:
  - Standalone command-line executable compiled natively without loading Qt GUI modules.
  - Supports `search`, `info`, `tail`, `bench` subcommands, plus `--version` and `--help`.
  - Enables headless integration into CI/CD pipelines, remote Windows shells, and PowerShell automation.

---

### 3.3. Presentation Layer & Models (`source/model/` and `source/ui/`)

- **`LogModel.hpp/.cpp`**:
  - Inherits from `QAbstractTableModel`. Avoids storing line copies in Qt heap memory;
  - The `data(const QModelIndex&, int role)` method reads text slices directly from mapped memory using `ChunkIndex` offsets, decoding and rendering on demand;
  - Supports filtered view mode exposing solely query-matching records.
- **`MainWindow.hpp/.cpp`**:
  - Centralizes the `StickyMenu` bar, top action toolbar, splitter panels (`QSplitter`), and status bar;
  - Listens to native Windows events (`WM_SETTINGCHANGE`, `nativeEvent`) for real-time theme and palette synchronization.
- **`LogView.hpp/.cpp`**:
  - Customized `QTableView` engineered for extreme rendering fluidity, pixel-smooth scrolling, and responsive text selection;
  - Direct integration with the custom `MinimapScrollBar`.
- **`MinimapScrollBar.hpp/.cpp`**:
  - Renders an analytical minimap in the vertical scroll trough, mapping linear proportions of the file and painting color-coded markers for errors, warnings, and search hits.
- **`SearchBarWidget.hpp/.cpp`**:
  - Integrated search toolbar supporting regex, case sensitivity, whole-word matching, occurrence navigation, and filter mode toggling.
- **`TimeSliceWidget.hpp/.cpp`**:
  - Interactive bar histogram widget enabling users to inspect event density over time and filter specific intervals with a single click.
- **`JsonInspectorWidget.hpp/.cpp`**:
  - Inspector detecting and formatting JSON payloads found on selected rows, displaying properties in an expandable tree with quick-copy actions.
- **`LogGeneratorDialog.hpp/.cpp`**:
  - Synthetic log generator dialog creating 100 MB, 500 MB, and 1 GB test files for benchmarking and stress testing.
- **`ThemeManager.hpp/.cpp` and `Win32DwmHelper.hpp/.cpp`**:
  - Centralized manager for Qt light and dark palettes;
  - Interacts with Windows DWM APIs for immersive dark/light title bar integration (`DwmSetWindowAttribute`).
- **`GerenciadorDialogos.hpp/.cpp`**:
  - Governs lifecycle and Z-order management for `SobreDialog` and `ManualDialog`, ensuring modeless window decoupling.

---

### 3.4. Internationalization (`source/language/`)

- **`GerenciadorTraducao.hpp/.cpp`**:
  - Manages installation and unloading of `QTranslator` catalogs at runtime.
  - Dynamically loads compiled catalogs:
    - `source/language/translations/munin_pt_BR.qm`
    - `source/language/translations/munin_en_US.qm`
  - Propagates `QEvent::LanguageChange` events so all UI widgets, tooltips, menus, and dialogs update instantly.

---

### 3.5. Mocks & Developer Utilities (`mocks/`)

- **`find_VersionEditor.cpp/.hpp`**:
  - Graphical and CLI tool for synchronous validation and updating of version and date across all 28 critical project files:
    - `source/assets/ABOUT/ABOUT_pt_BR.txt` and `ABOUT_en_US.txt`
    - `source/assets/CLC/CLC_pt_BR.txt` and `CLC_en_US.txt`
    - `source/assets/EULA/EULA_pt_BR.txt` and `EULA_en_US.txt`
    - `source/assets/NOTICES/NOTICE_pt_BR.txt` and `NOTICE_en_US.txt`
    - `source/assets/PRIVACY_POLICY/Privacy_Policy_pt_BR.txt` and `Privacy_Policy_en_US.txt`
    - `main.cpp`, `source/main.cpp`, `source/cli/main_cli.cpp`
    - `source/ui/SobreDialog.cpp`, `source/ui/GerenciadorDialogos.cpp`
    - `README.md`, `MANUAL.md`, `OBSERVACAO.md`
    - `docs/architecture.md`, `docs/cli-reference.md`, `docs/compliance.md`, `docs/licensing.md`, `docs/security-model.md`, `docs/user-guide.md`, `docs/release-v2026.11.6.0.md`
    - `repo/Munin.iss`, `repo/Munin_MinGW.iss`, and `repo/Munin_MSVC.iss`
  - CLI integrity verification: `find_VersionEditor.exe --check`
  - CLI synchronous update: `find_VersionEditor.exe --update [folder] [version]`
- **`CompileTranslationsGUI.cpp` and `CompileTranslationsTerminal.cpp`**:
  - Utilities for automated compilation of translation `.ts` source files into `.qm` binaries.

---

## 4. Local Persistence & Storage

Munin App strictly adheres to Windows desktop and enterprise storage guidelines:

- **Root Settings Directory**:
  - `%LOCALAPPDATA%/Munin/`
- **Persisted Configuration**:
  - `config.ini` or settings via `QSettings`:
    - Active user-selected language (`pt_BR` or `en_US`);
    - Main window geometry and state (size, position, maximized flag, splitter layout);
    - Visual toolbar and panel preferences;
- **Temporary Files**:
  - The application inspects files directly from their original location via memory mapping, generating zero redundant temporary disk files.

---

## 5. CMake Build, Presets, and Targets

The project requires CMake 3.21+ and features declarative presets in `CMakePresets.json`:

### 5.1. Configuration & Build Presets

| Preset | Compiler | Type | Description |
| :--- | :--- | :--- | :--- |
| `host-qt6-mingw-release` | MinGW GCC 13.1.0+ | Release | Builds Munin App using MinGW toolchain and Qt 6.11.1 runtime (Ninja). |
| `host-qt6-msvc-release` | MSVC 2022 v143+ | Release | Builds Munin App using MSVC toolchain and Qt 6.11.1 runtime (Ninja). |
| `host-qt6-bench-mingw-release` | MinGW GCC 13.1.0+ | Release | Builds MuninBench benchmark using MinGW. |
| `host-qt6-bench-msvc-release` | MSVC 2022 v143+ | Release | Builds MuninBench benchmark using MSVC. |
| `host-qt6-mocks-mingw-release` | MinGW GCC 13.1.0+ | Release | Builds support utilities and mocks (find_VersionEditor) via MinGW. |
| `host-qt6-mocks-msvc-release` | MSVC 2022 v143+ | Release | Builds support utilities and mocks (find_VersionEditor) via MSVC. |

### 5.2. Primary Compilation Targets

- **`MuninEngine`**: Static library compiling C++20 low-level engine components.
- **`Munin`**: Primary Qt6 desktop graphical application.
- **`munin-app-cli`**: Standalone command-line executable built from `source/cli/main_cli.cpp`.
- **`MuninBench`**: Command-line executable for I/O and SIMD benchmarks.
- **`MuninDeploy`**: Custom target automating `windeployqt.exe`, copying native plugins (styles, platforms, svg), `.qm` translation catalogs, assets, and synchronizing external clean distributions.
- **`find_VersionEditor` / `find_VersionEditorDeploy`**: Developer utility targets.

### 5.3. Clean Distributions

Following compilation with target `MuninDeploy`, clean and self-contained distributions are generated in:
- `D:\MISCELANEAS\VSCode\Munin\Munin_mingw\`
- `D:\MISCELANEAS\VSCode\Munin\Munin_msvc\`

Both contain `Munin.exe`, `munin-app-cli.exe`, `MuninBench.exe`, along with all required runtime dependencies and assets.

---

## 6. Packaging with Inno Setup and MSIX

For generating professional Windows installers, the project provides scripts in the `repo/` folder:

- **`repo/Munin.iss`**: Master unified script compiling both MinGW and MSVC editions via `/DBuildFlavor=mingw` or `/DBuildFlavor=msvc`.
- **`repo/Munin_MinGW.iss`**: Script dedicated to packaging the MinGW distribution (`Munin_mingw_v2026.11.6.0.exe`).
- **`repo/Munin_MSVC.iss`**: Script dedicated to packaging the MSVC distribution (`Munin_msvc_v2026.11.6.0.exe`).
- **`repo/AppxManifest.xml`**: Microsoft Store MSIX manifest configuring `AppExecutionAlias` for `munin` and `munin-app-cli`.
- **Installer Output Directory**:
  - `D:\MISCELANEAS\VSCode\Munin\InnoSetupOutput\`

---

## 7. Conclusion

The **Munin_App_cpp** project is in a mature, stable, and fully operational state at version **`2026.10.18.0`**, with validated compilation and testing across both **MinGW** and **MSVC** environments, complete support for desktop GUI and headless CLI execution, 100% automated integrity validation across all monitored files via `find_VersionEditor --check`, and automated generation of clean distributions and installers.

</details>

---

<h2 id="tree">🌳 Árvore de Arquivos / File Tree</h2>

`./`

```
.gitattributes
.gitignore
CMakeLists.txt
CMakePresets.json
CODE_OF_CONDUCT.md
CONTRIBUTING.md
LICENSE
main.cpp
MANUAL.md
OBSERVACAO.md
README.md
SECURITY.md
rascunho.cpp
```

**Root Path:** `./source`

```
├── 📁 assets
│   ├── 📁 ABOUT
│   │   ├── 📄 ABOUT_en_US.txt
│   │   ├── 📄 ABOUT_pt_BR.txt
│   │   ├── 📄 History_APP_en_US.txt
│   │   └── 📄 History_APP_pt_BR.txt
│   ├── 📁 CLC
│   │   ├── 📄 CLC_en_US.txt
│   │   └── 📄 CLC_pt_BR.txt
│   ├── 📁 COPYRIGHT
│   │   ├── 📄 AVISO DE COPYRIGHT E MARCA REGISTRADA_pt_BR.txt
│   │   └── 📄 COPYRIGHT AND TRADEMARK NOTICE_en_US.txt
│   ├── 📁 EULA
│   │   ├── 📄 EULA_en_US.txt
│   │   └── 📄 EULA_pt_BR.txt
│   ├── 📁 LICENSES
│   │   ├── 📄 Apache License Version 2.0.txt
│   │   ├── 📄 BSD License Clause-2.txt
│   │   ├── 📄 BSD License Clause-3.txt
│   │   ├── 📄 GNU AFFERO GENERAL PUBLIC LICENSE Version3.txt
│   │   ├── 📄 GNU GENERAL PUBLIC LICENSE Version 3.txt
│   │   ├── 📄 GNU LESSER GENERAL PUBLIC LICENSE Version 2.1.txt
│   │   ├── 📄 GNU LESSER GENERAL PUBLIC LICENSE Version 3.txt
│   │   ├── 📄 ISC LICENSE.txt
│   │   ├── 📄 License.txt
│   │   ├── 📄 MIT License.txt
│   │   └── 📄 PYTHON SOFTWARE FOUNDATION LICENSE.txt
│   ├── 📁 NOTICES
│   │   ├── 📄 NOTICE_en_US.txt
│   │   └── 📄 NOTICE_pt_BR.txt
│   ├── 📁 PRIVACY_POLICY
│   │   ├── 📄 Privacy_Policy_en_US.txt
│   │   └── 📄 Privacy_Policy_pt_BR.txt
│   ├── 📁 RELEASE
│   │   ├── 📄 RELEASE NOTES_en_US.txt
│   │   └── 📄 RELEASE NOTES_pt_BR.txt
│   └── 📁 icons
│       ├── 📄 munin.ico
│       ├── 🖼️ munin.png
│       ├── 🖼️ munin_1080-1080.png
│       ├── 🖼️ munin_1440-2160.png
│       ├── 🖼️ munin_150-150.png
│       ├── 🖼️ munin_1920-1080.png
│       ├── 🖼️ munin_2160-2160.png
│       ├── 🖼️ munin_300-300.png
│       ├── 🖼️ munin_3840-2160.png
│       ├── 🖼️ munin_584-800.png
│       └── 🖼️ munin_71-71.png
├── 📁 cli
│   └── ⚡ main_cli.cpp
├── 📁 engine
│   ├── ⚡ ChunkIndex.cpp
│   ├── ⚡ ChunkIndex.hpp
│   ├── ⚡ LiveTailWatcher.cpp
│   ├── ⚡ LiveTailWatcher.hpp
│   ├── ⚡ MemoryMappedFile.cpp
│   ├── ⚡ MemoryMappedFile.hpp
│   ├── ⚡ ParallelSearch.cpp
│   ├── ⚡ ParallelSearch.hpp
│   ├── ⚡ SimdUtils.cpp
│   ├── ⚡ SimdUtils.hpp
│   ├── ⚡ TestEngine.cpp
│   ├── ⚡ TimeSliceScanner.cpp
│   └── ⚡ TimeSliceScanner.hpp
├── 📁 language
│   ├── 📁 translations
│   │   ├── 📄 munin_en_US.qm
│   │   ├── 📄 munin_en_US.ts
│   │   ├── 📄 munin_pt_BR.qm
│   │   └── 📄 munin_pt_BR.ts
│   ├── ⚡ GerenciadorTraducao.cpp
│   └── ⚡ GerenciadorTraducao.hpp
├── 📁 model
│   ├── ⚡ LogModel.cpp
│   └── ⚡ LogModel.hpp
├── 📁 ui
│   ├── ⚡ ButtonUtils.cpp
│   ├── ⚡ ButtonUtils.hpp
│   ├── ⚡ GerenciadorDialogos.cpp
│   ├── ⚡ GerenciadorDialogos.hpp
│   ├── ⚡ JsonInspectorWidget.cpp
│   ├── ⚡ JsonInspectorWidget.hpp
│   ├── ⚡ LogGeneratorDialog.cpp
│   ├── ⚡ LogGeneratorDialog.hpp
│   ├── ⚡ LogItemDelegate.cpp
│   ├── ⚡ LogItemDelegate.hpp
│   ├── ⚡ LogView.cpp
│   ├── ⚡ LogView.hpp
│   ├── ⚡ MainWindow.cpp
│   ├── ⚡ MainWindow.hpp
│   ├── ⚡ Manual.cpp
│   ├── ⚡ Manual.hpp
│   ├── ⚡ MinimapScrollBar.cpp
│   ├── ⚡ MinimapScrollBar.hpp
│   ├── ⚡ OpcoesSobre.cpp
│   ├── ⚡ OpcoesSobre.hpp
│   ├── ⚡ SearchBarWidget.cpp
│   ├── ⚡ SearchBarWidget.hpp
│   ├── ⚡ SobreDialog.cpp
│   ├── ⚡ SobreDialog.hpp
│   ├── ⚡ StickyMenu.cpp
│   ├── ⚡ StickyMenu.hpp
│   ├── ⚡ ThemeManager.cpp
│   ├── ⚡ ThemeManager.hpp
│   ├── ⚡ TimeSliceWidget.cpp
│   ├── ⚡ TimeSliceWidget.hpp
│   ├── ⚡ Win32DwmHelper.cpp
│   └── ⚡ Win32DwmHelper.hpp
├── 📁 utils
│   ├── ⚡ ApplicationPathUtils.cpp
│   ├── ⚡ ApplicationPathUtils.hpp
│   ├── ⚡ CaminhoPersistenteUtils.cpp
│   ├── ⚡ CaminhoPersistenteUtils.hpp
│   ├── ⚡ LogManager.cpp
│   └── ⚡ LogManager.hpp
└── ⚡ main.cpp
```

**Root Path:** `./mocks`

```
├── 📁 icons
│   ├── 📄 munin.ico
│   ├── 🖼️ munin.png
│   ├── 🖼️ munin_1080-1080.png
│   ├── 🖼️ munin_1440-2160.png
│   ├── 🖼️ munin_150-150.png
│   ├── 🖼️ munin_1920-1080.png
│   ├── 🖼️ munin_2160-2160.png
│   ├── 🖼️ munin_300-300.png
│   ├── 🖼️ munin_3840-2160.png
│   ├── 🖼️ munin_584-800.png
│   └── 🖼️ munin_71-71.png
├── 📄 CMakeLists.txt
├── ⚡ CompileTranslationsGUI.cpp
├── ⚡ CompileTranslationsGUI.hpp
├── ⚡ CompileTranslationsTerminal.cpp
├── ⚡ CompileTranslationsTerminal.hpp
├── ⚡ find_VersionEditor.cpp
├── ⚡ find_VersionEditor.hpp
```

**Root Path:** `./docs`

```
├── 📝 architecture.md
├── 📝 cli-reference.md
├── 📝 compliance.md
├── 📝 licensing.md
├── 📝 release-v2026.11.6.0.md
├── 📝 security-model.md
├── 📝 suggested-issues.md
└── 📝 user-guide.md
```

**Root Path:** `./.github`

```
├── 📁 ISSUE_TEMPLATE
│   ├── 📝 bug_report.md
│   ├── ⚙️ config.yml
│   ├── 📝 feature_request.md
│   └── 📝 question.md
├── 📁 workflows
│   └── ⚙️ update-readme-version.yml
└── 📝 PULL_REQUEST_TEMPLATE.md
```

**Root Path:** `./repo/LISTAGEM`

```
├── 📁 en_US
│   ├── 🖼️ Captura de tela 2026-10-17 024044.png
│   ├── 🖼️ Captura de tela 2026-10-17 024121.png
│   └── 🖼️ Captura de tela 2026-10-17 024141.png
└── 📁 pt_BR
    ├── 🖼️ Captura de tela 2026-10-17 023534.png
    ├── 🖼️ Captura de tela 2026-10-17 023653.png
    └── 🖼️ Captura de tela 2026-10-17 023713.png
```
