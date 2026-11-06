# Arquitetura do Sistema / System Architecture — Munin App

<p align="center">
  <b>Selecione o idioma / Select language:</b><br>
  <a href="#ptbr">🇧🇷 Português (BR)</a> | <a href="#enus">🇺🇸 English (US)</a>
</p>

---

## <a id="ptbr"></a>🇧🇷 Português (BR)

<details>
<summary>Clique para expandir / recolher a arquitetura em português</summary><br>

# Munin App — Arquitetura de Sistema e Especificações Técnicas

**Versão:** `2026.11.6.0`  
**Data:** `6 de novembro de 2026`  
**Padrão:** C++20 | Qt 6.11.1 | Win32 Memory API | Vetorização SIMD (AVX2/SSE4.2)  
**Plataforma Alvo:** Windows 11 / Windows 10 (x64)  
**Autor:** Fernando Nillsson Cidade  

---

## 1. Filosofia Arquitetural

O **Munin App** foi projetado para resolver o problema crítico do esgotamento de memória RAM e travamento de editores convencionais ao abrir arquivos de log massivos (centenas de megabytes a dezenas de gigabytes). Sua engenharia fundamenta-se em quatro pilares estritos:

1. **Zero-Copy e Mapeamento Virtual Win32:** Nenhum arquivo é carregado integralmente na memória heap do processo. Páginas virtuais são gerenciadas sob demanda diretamente pelo subsistema de paginação do kernel do Windows NT.
2. **Desacoplamento Estrito da Engine (`MuninEngine`):** O motor de processamento, leitura, vetorização SIMD e busca é compilado como biblioteca estática C++20 pura, sem qualquer dependência de bibliotecas de interface gráfica (sem Qt).
3. **Navegação Virtualizada de Alta Taxa de Quadros (60 FPS):** A camada visual renderiza apenas as linhas visíveis na viewport atual, mantendo o consumo de memória fixo em poucos megabytes independentemente do tamanho do arquivo (seja 50 MB ou 50 GB).
4. **Local-First e Privacidade Absoluta:** Toda a análise ocorre 100% offline no dispositivo local, sem conexões de rede, sem telemetria e sem dependência de serviços externos.

```
+-------------------------------------------------------------------------+
|                           CAMADA DE APRESENTAÇÃO                        |
|                     (Qt 6.11.1 Widgets, DWM 11 Dark / Mica)             |
|                                                                         |
|  MainWindow  |  LogView (Tabela Virtual)  |  MinimapScrollBar           |
|  SearchBarWidget  |  TimeSliceWidget (Timeline) | JsonInspectorWidget   |
|  LogGeneratorDialog  |  GerenciadorDialogos (Sobre / Manual)            |
+-------------------------------------------------------------------------+
                                    |
                                    v
+-------------------------------------------------------------------------+
|                            MODELOS VIRTUAIS                             |
|                       (Qt Model/View QAbstractTableModel)               |
|                                                                         |
|    LogModel (Mapeamento Virtual JIT / Paginação sob Demanda de Linhas)  |
+-------------------------------------------------------------------------+
                                    |
                                    v
+-------------------------------------------------------------------------+
|                        ENGINE C++20 DE BAIXO NÍVEL                      |
|                  (MuninEngine - Independente de GUI / C++20)            |
|                                                                         |
|  +------------------------+  +---------------------------------------+  |
|  |  MemoryMappedFile      |  |  SimdUtils                            |  |
|  |  - CreateFileMappingW  |  |  - AVX2 256-bit (_mm256_movemask_epi8)|  |
|  |  - MapViewOfFile 64-bit|  |  - SSE4.2 128-bit (_mm_movemask_epi8) |  |
|  |  - Leitura Compartilhada| |  - Fallback Escalar Otimizado         |  |
|  +------------------------+  +---------------------------------------+  |
|  +------------------------+  +---------------------------------------+  |
|  |  ChunkIndex            |  |  ParallelSearch                       |  |
|  |  - Chunks de 64 KB     |  |  - Multithread (Hardware Threads)     |  |
|  |  - Acesso Aleatório O(1)| |  - Regex JIT / Filtro Dinâmico        |  |
|  +------------------------+  +---------------------------------------+  |
|  +------------------------+  +---------------------------------------+  |
|  |  LiveTailWatcher       |  |  TimeSliceScanner                     |  |
|  |  - ReadDirectoryChanges|  |  - Parser ISO-8601                    |  |
|  |  - Notificações Kernel |  |  - Histograma Temporal                |  |
|  +------------------------+  +---------------------------------------+  |
+-------------------------------------------------------------------------+
```

---

## 2. Componentes da Engine (`MuninEngine`)

### 2.1. Mapeamento Virtual Win32 (`MemoryMappedFile.cpp`)
- **Chamadas de Sistema:** `CreateFileW` com `GENERIC_READ` e permissão total de compartilhamento (`FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE`).
- **Espaço de Endereçamento:** `CreateFileMappingW` e `MapViewOfFile` no espaço virtual de 64 bits.
- **Vantagem:** Latência de abertura inferior a 1 milissegundo mesmo para arquivos de 20 GB+.
- **RAII Estrito:** Encapsulamento automático de `HANDLE` e ponteiros de mapeamento com `UnmapViewOfFile` e `CloseHandle`.

### 2.2. Indexação Vetorial por Instruções SIMD (`SimdUtils.cpp`)
- **Detecção em Tempo de Execução:** Identificação via `__cpuid` das instruções disponíveis no processador host.
- **AVX2 (256 bits):** Escaneamento de delimitadores de quebra de linha `\n` processando 32 bytes por ciclo de CPU utilizando `_mm256_loadu_si256`, `_mm256_cmpeq_epi8` e `_mm256_movemask_epi8`.
- **SSE4.2 (128 bits):** Fallback processando 16 bytes por ciclo com `_mm_loadu_si128` e `_mm_movemask_epi8`.
- **Fallback Escalar:** Laço otimizado para plataformas sem suporte vetorial avançado.
- **Rendimento:** Velocidades de escaneamento superiores a 3 GB/s em SSDs NVMe modernos.

### 2.3. Indexação Esparsa por Chunks (`ChunkIndex.cpp`)
- **Tamanho dos Blocos:** Particionamento esparso em blocos de 64 KB.
- **Complexidade de Acesso:** $O(1)$ para localizar qualquer linha por meio de offsets pré-calculados sem alocar strings completas na memória heap.

### 2.4. Monitoramento Live Tail (`LiveTailWatcher.cpp`)
- **Notificação de Kernel:** Utiliza a API Win32 `ReadDirectoryChangesW` em thread de segundo plano.
- **Consumo de CPU:** Zero consumo ocioso de processador (sem laços de polling).
- **Expansão Incremental:** Novos dados anexados são indexados e refletidos na interface em tempo real com auto-scroll inteligente.

### 2.5. Motor de Busca Paralela Multi-Core (`ParallelSearch.cpp`)
- **Divisão de Trabalho:** Partição automática dos chunks do arquivo entre todos os núcleos de hardware (`std::thread::hardware_concurrency()`).
- **Modos de Busca:** Busca textual literal, sensibilidade a maiúsculas/minúsculas, correspondência de palavras inteiras e Expressões Regulares completas (PCRE2 JIT).
- **Modo Filtragem:** Permite isolar e exibir continuamente apenas as linhas correspondentes à consulta.

### 2.6. Analisador Temporal (`TimeSliceScanner.cpp`)
- **Extração de Timestamps:** Reconhecimento automático de formatos padronizados (ISO-8601 e variações comuns).
- **Geração de Histograma:** Distribuição de contagem de eventos por intervalo de tempo para visualização em linha do tempo.

---

## 3. Camada de Apresentação e Componentes Gráficos (Qt6)

- **`LogModel`:** Modelo tabular que atua como ponte virtual com a `MuninEngine`. As linhas de texto são requisitadas à engine e convertidas temporariamente para a viewport apenas no momento da pintura.
- **`LogView`:** Visualizador virtualizado customizado derivado de `QTableView`, oferecendo rolagem suave e seleção contínua.
- **`MinimapScrollBar`:** Barra de rolagem lateral com mapa visual de densidade de eventos e marcadores cromáticos (vermelho para erros, amarelo para avisos e azul/ciano para ocorrências de busca).
- **`TimeSliceWidget`:** Componente gráfico interativo com gráfico de barras temporal e filtros rápidos por período.
- **`JsonInspectorWidget`:** Painel retrátil lateral que analisa linhas selecionadas, identificando payloads JSON e desenhando uma árvore estruturada de nós com realce sintático.
- **`LogGeneratorDialog`:** Ferramenta integrada de síntese local de arquivos de teste (100 MB, 500 MB e 1 GB) para baterias de testes e homologação de taxa de transferência.
- **`ThemeManager`:** Gerenciador dinâmico de temas sincronizado com o Windows 11 DWM (Mica / Dark Title Bar).
- **`GerenciadorDialogos`:** Controlador de janelas modeless desacopladas com Z-Order independente (`SobreDialog`, `ManualDialog`).

---

## 4. Ferramentas Auxiliares e Testes

- **`MuninBench`:** Utilitário autônomo de linha de comando (`source/engine/TestEngine.cpp`) para testes rigorosos de taxa de transferência de memória mapeada, busca paralela e velocidade de indexação SIMD.
- **`munin-app-cli`:** Utilitário standalone de linha de comando (`source/cli/main_cli.cpp`) para automação, busca paralela, extração de metadados de logs e streaming live tail sem inicialização gráfica.
- **`find_VersionEditor`:** Utilitário gráfico e de linha de comando (`mocks/find_VersionEditor.cpp`) responsável por auditar e sincronizar as versões e datas de todos os 28 arquivos do ecossistema Munin App.

</details>

---

## <a id="enus"></a>🇺🇸 English (US)

<details>
<summary>Click to expand / collapse architecture in English</summary><br>

# Munin App — System Architecture and Technical Specifications

**Version:** `2026.11.6.0`  
**Date:** `November 6, 2026`  
**Standards:** C++20 | Qt 6.11.1 | Win32 Memory API | SIMD Vectorization (AVX2/SSE4.2)  
**Target Platform:** Windows 11 / Windows 10 (x64)  
**Author:** Fernando Nillsson Cidade  

---

## 1. Architectural Philosophy

**Munin App** is engineered to eliminate memory exhaustion and UI freezes when opening massive log archives (hundreds of megabytes to tens of gigabytes). Its design is built on four core principles:

1. **Zero-Copy & Win32 Virtual Paging:** Log files are never fully loaded into heap RAM. Pages are demand-mapped directly via the Windows NT virtual memory manager.
2. **Strict Engine Decoupling (`MuninEngine`):** Core I/O, SIMD scanning, and search algorithms are built as an isolated C++20 static library, completely independent of Qt and GUI frameworks.
3. **High Framerate Virtual Navigation (60 FPS):** Only lines currently in viewport are formatted and painted, maintaining a fixed low-memory footprint regardless of file size.
4. **Local-First & Complete Privacy:** All analysis happens 100% offline on the host workstation. No network sockets, telemetry, or external cloud services.

```
+-------------------------------------------------------------------------+
|                           PRESENTATION LAYER                            |
|                     (Qt 6.11.1 Widgets, DWM 11 Dark / Mica)             |
|                                                                         |
|  MainWindow  |  LogView (Virtual Table)   |  MinimapScrollBar           |
|  SearchBarWidget  |  TimeSliceWidget (Timeline) | JsonInspectorWidget   |
|  LogGeneratorDialog  |  GerenciadorDialogos (About / Manual)            |
+-------------------------------------------------------------------------+
                                    |
                                    v
+-------------------------------------------------------------------------+
|                             VIRTUAL MODELS                              |
|                       (Qt Model/View QAbstractTableModel)               |
|                                                                         |
|    LogModel (JIT Virtual Memory Mapping / Demand Paging of Rows)        |
+-------------------------------------------------------------------------+
                                    |
                                    v
+-------------------------------------------------------------------------+
|                        LOW-LEVEL C++20 ENGINE                           |
|                  (MuninEngine - GUI-Independent / C++20)                |
|                                                                         |
|  +------------------------+  +---------------------------------------+  |
|  |  MemoryMappedFile      |  |  SimdUtils                            |  |
|  |  - CreateFileMappingW  |  |  - AVX2 256-bit (_mm256_movemask_epi8)|  |
|  |  - MapViewOfFile 64-bit|  |  - SSE4.2 128-bit (_mm_movemask_epi8) |  |
|  |  - Shared Read Access  |  |  - Optimized Scalar Fallback          |  |
|  +------------------------+  +---------------------------------------+  |
|  +------------------------+  +---------------------------------------+  |
|  |  ChunkIndex            |  |  ParallelSearch                       |  |
|  |  - 64 KB Chunks        |  |  - Multithreaded (Hardware Threads)   |  |
|  |  - O(1) Random Access  |  |  - JIT Regex / Dynamic Row Filter     |  |
|  +------------------------+  +---------------------------------------+  |
|  +------------------------+  +---------------------------------------+  |
|  |  LiveTailWatcher       |  |  TimeSliceScanner                     |  |
|  |  - ReadDirectoryChanges|  |  - ISO-8601 Parser                    |  |
|  |  - Kernel Notifications|  |  - Temporal Event Histogram           |  |
|  +------------------------+  +---------------------------------------+  |
+-------------------------------------------------------------------------+
```

---

## 2. Engine Components (`MuninEngine`)

### 2.1. Win32 Virtual Memory Mapping (`MemoryMappedFile.cpp`)
- **Native System Calls:** `CreateFileW` with `GENERIC_READ` and full sharing (`FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE`).
- **Address Space:** `CreateFileMappingW` and `MapViewOfFile` in 64-bit virtual address space.
- **Latency:** Instantaneous opening (< 1 ms) even for 20 GB+ files.
- **Strict RAII:** Automatic handle lifecycle management with `UnmapViewOfFile` and `CloseHandle`.

### 2.2. SIMD Vectorized Indexing (`SimdUtils.cpp`)
- **Runtime CPU Feature Detection:** Identifies available vector extensions via `__cpuid`.
- **AVX2 (256-bit):** Scans newline delimiters (`\n`) across 32 bytes per clock cycle using `_mm256_loadu_si256`, `_mm256_cmpeq_epi8`, and `_mm256_movemask_epi8`.
- **SSE4.2 (128-bit):** Fallback scanning 16 bytes per cycle via `_mm_loadu_si128` and `_mm_movemask_epi8`.
- **Scalar Fallback:** Optimized byte-by-byte loop for legacy hardware.
- **Throughput:** In excess of 3 GB/s on modern NVMe SSDs.

### 2.3. Sparse Chunk Indexing (`ChunkIndex.cpp`)
- **Block Granularity:** Partitioned in 64 KB sparse chunks.
- **Access Complexity:** $O(1)$ random line access through pre-calculated offset tables without heap allocations.

### 2.4. Live Tail Monitoring (`LiveTailWatcher.cpp`)
- **Kernel Notifications:** Win32 `ReadDirectoryChangesW` running on a dedicated background worker thread.
- **CPU Overhead:** Zero idle CPU consumption (no polling loops).
- **Incremental Expansion:** Newly appended bytes are indexed on-the-fly and displayed in real-time.

### 2.5. Multi-Core Parallel Search (`ParallelSearch.cpp`)
- **Workload Partitioning:** Log file chunks are distributed evenly across all hardware cores (`std::thread::hardware_concurrency()`).
- **Search Capabilities:** Literal text, case sensitivity, word boundaries, and compiled PCRE2 JIT regular expressions.
- **Filter Mode:** Isolates and displays only matching rows continuously.

### 2.6. Temporal Analysis (`TimeSliceScanner.cpp`)
- **Timestamp Extraction:** Automatic parsing of standard timestamp formats (ISO-8601).
- **Histogram Generation:** Log event density aggregated by time slice.

---

## 3. UI Presentation Layer (Qt6)

- **`LogModel`:** Virtualized table model bridging `MuninEngine` and the view. Rows are pulled and materialized only when requested by the painter.
- **`LogView`:** High-performance virtualized table widget derived from `QTableView`.
- **`MinimapScrollBar`:** Density heatmap scrollbar with color-coded markers (red for errors, yellow for warnings, cyan for search hits).
- **`TimeSliceWidget`:** Interactive temporal histogram widget with one-click time range filtering.
- **`JsonInspectorWidget`:** Collapsible side panel parsing selected log lines and presenting structured JSON trees.
- **`LogGeneratorDialog`:** Synthetic test log generator (100 MB, 500 MB, 1 GB) for performance validation.
- **`ThemeManager`:** Dynamic theme manager integrated with Windows 11 DWM (Mica and dark title bar).
- **`GerenciadorDialogos`:** Modeless dialog manager orchestrating independent Z-order windows (`SobreDialog`, `ManualDialog`).

---

## 4. Tools & Benchmarking

- **`MuninBench`:** Standalone command-line benchmark utility (`source/engine/TestEngine.cpp`) measuring SIMD throughput and multi-threaded search speed.
- **`munin-app-cli`:** Standalone command-line interface (`source/cli/main_cli.cpp`) for headless log search, metadata extraction, live tailing, and CI/CD automation without launching the GUI.
- **`find_VersionEditor`:** GUI and CLI mock utility (`mocks/find_VersionEditor.cpp`) managing version synchronization across all 28 repository files.

</details>
