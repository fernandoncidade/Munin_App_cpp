# MANUAL - Munin App

Versão documentada: `2026.11.6.0`  
Data: `6 de novembro de 2026`  
Version documented: `2026.11.6.0`  
Date: `November 6, 2026`  

<p align="center">
  <b>Selecione o idioma / Select language:</b><br>
  <a href="#ptbr">Português (Brasil)</a> |
  <a href="#enus">English (United States)</a>
</p>

---

## <a id="ptbr"></a>Português (Brasil)

> Este manual descreve o funcionamento operacional, os componentes de interface, a arquitetura interna e os fluxos de trabalho do **Munin App — Leitor de Logs Massivos**.

<details>
<summary>Clique para expandir / recolher o MANUAL em português</summary>

### 1. Visão Geral e Princípios Arquiteturais

O **Munin App** é um visualizador e analisador de arquivos de log de alto rendimento para a plataforma Windows 11, desenvolvido em **C++20** com interface gráfica em **Qt 6**. Ele foi concebido para resolver o gargalo de visualizadores e editores de texto convencionais, que travam ou esgotam a memória do sistema ao tentar abrir arquivos com centenas de megabytes ou dezenas de gigabytes.

#### Princípios Fundamentais:
1. **Mapeamento em Memória Virtual Win32 (*Memory-Mapped Files*)**:
   - Em vez de ler todo o arquivo para a memória RAM heap, o Munin App solicita ao kernel do Windows NT a criação de uma seção mapeada (`CreateFileMappingW` e `MapViewOfFile`).
   - As páginas de dados são transferidas do disco diretamente para o cache do sistema operacional sob demanda (*page fault* assistido por hardware), permitindo abrir arquivos de 1 GB, 10 GB ou centenas de gigabytes em milissegundos com consumo ínfimo de memória RAM física.
2. **Indexação Vetorial por Instruções SIMD (AVX2 e SSE4.2)**:
   - A indexação inicial do arquivo escaneia delimitadores de linha (`\n`) utilizando instruções vetoriais de 256 bits (`_mm256_movemask_epi8`) e 128 bits (`_mm_movemask_epi8`), com fallback escalar inteligente.
   - A taxa de processamento atinge mais de 3 GB/s, mapeando milhões de linhas por segundo.
3. **Índice Esparso por Chunks (`ChunkIndex`)**:
   - O arquivo é subdividido em blocos virtuais de 64 KB com tabelas de offsets de linha. Isso garante busca binária e acesso $O(1)$ a qualquer posição do documento.
4. **Monitoramento Live Tail Assíncrono (`LiveTailWatcher`)**:
   - Acompanhamento contínuo de escritas concorrentes por meio da API Win32 `ReadDirectoryChangesW`, sem loops de espera ativa (*polling*).
5. **Busca Paralela Multi-Core (`ParallelSearch`)**:
   - Distribuição de blocos de pesquisa entre todos os núcleos lógicos da CPU com suporte a expressões regulares JIT, busca literal e filtragem de registros.
6. **Soberania e Privacidade Absoluta**:
   - Execução 100% autônoma e offline. Nenhum dado, caminho de arquivo, termo pesquisado ou log é compartilhado ou transmitido.

---

### 2. Interface Principal e Elementos Visuais

A interface do Munin App é organizada para prover máxima área útil de leitura e resposta imediata:

```text
+-----------------------------------------------------------------------------------+
|  Arquivo    Exibir    Configurações    Sobre                         [ _ ] [ 口 ] [ X ]|
+-----------------------------------------------------------------------------------+
| [ Abrir ] [ Live Tail ] [ Busca ] [ Timeline ] [ JSON ] [ Gerador ] | Filtro: Ativo |
+-----------------------------------------------------------------------------------+
| [ Busca Rápida: [Texto ou Regex...]  (Aa) (Palavra) (.*)  [< Ant] [Próx >] (X) ]  |
+----------------------------------------------------------+------------------------+
| Linha | Registro de Log                                  |     MINIMAPA DE        |
|-------+--------------------------------------------------|      DENSIDADE         |
| 1     | 2026-10-14 10:00:01.123 [INFO] Servidor iniciado |                        |
| 2     | 2026-10-14 10:00:02.456 [DEBUG] Conexao aceita   |       [   ]            |
| 3     | 2026-10-14 10:00:05.789 [WARN] Latencia alta     |       [ # ] -> Erro    |
| 4     | 2026-10-14 10:00:10.012 [ERROR] Falha de conexao |       [ * ] -> Match   |
| ...   | ...                                              |                        |
+----------------------------------------------------------+------------------------+
| [ Histograma Time-Slice (Frequência por Hora / Minuto) ]                          |
+-----------------------------------------------------------------------------------+
| [ Inspetor de JSON Estruturado: Propriedades / Árvore Hierárquica ]               |
+-----------------------------------------------------------------------------------+
| Arquivo: app.log | Tamanho: 4.25 GB | Linhas: 38.542.100 | Velocidade: 3.42 GB/s  |
+-----------------------------------------------------------------------------------+
```

#### Componentes:
- **Barra de Menus Integrada (`StickyMenu`)**: Menus com resposta rápida e navegação por teclado (`Alt+A`, `Alt+E`, `Alt+C`, `Alt+B`).
- **Barra de Ferramentas Superior**: Botões de acesso rápido para abertura de arquivos, alternância de Live Tail, ativação de busca, linha do tempo, inspetor JSON e gerador de logs.
- **Visualizador Principal (`LogView`)**: Grid virtualizado que renderiza exclusivamente as linhas visíveis na janela, mantendo fluidez a 60+ FPS mesmo com dezenas de milhões de linhas carregadas.
- **Minimapa de Densidade (`MinimapScrollBar`)**: Barra de rolagem vertical personalizada que renderiza um mapa de calor visual do documento, evidenciando ocorrências de pesquisa (azul/ciano), avisos (amarelo) e erros (vermelho).
- **Barra de Status Inferior**: Informações em tempo real sobre o arquivo aberto:
  - Caminho do arquivo;
  - Tamanho formatado (B, KB, MB, GB);
  - Linhas totais e linhas visíveis (quando filtrado);
  - Velocidade de indexação/processamento (GB/s);
  - Consumo de memória RAM do processo;
  - Estado operacional do Live Tail (`Ativo` ou `Pausado`).

---

### 3. Abertura e Gerenciamento de Logs

#### Como abrir arquivos:
1. Pelo menu: **Arquivo > Abrir Arquivo...** (`Ctrl+O`).
2. Pelo botão **Abrir** na barra de ferramentas.
3. Por **Arrastar e Soltar (Drag & Drop)**: arraste um arquivo de log de qualquer tamanho diretamente do Windows Explorer para o centro da janela do Munin App.

#### Comportamento durante a abertura:
- O arquivo é mapeado em memória instantaneamente.
- Em segundo plano, a engine SIMD realiza o escaneamento vetorial dos delimitadores de nova linha, atualizando a barra de status com o total de linhas e a velocidade de processamento.
- A visualização é liberada imediatamente, permitindo que o usuário comece a navegar pela primeira página do log enquanto o restante do índice é concluído em background.

---

### 4. Monitoramento em Tempo Real (Live Tail)

O modo **Live Tail** replica o comportamento de ferramentas como `tail -f`, permitindo acompanhar gravações ativas em tempo real (como logs de servidores em execução ou testes de integração).

#### Como utilizar:
1. Pressione **Ctrl+L** ou clique no botão **Live Tail** na barra de ferramentas.
2. O botão passará a indicar o estado ativo com destaque visual.
3. À medida que novos registros forem gravados no disco pelo processo gerador do log, o Munin App:
   - Recebe a notificação de modificação do kernel via `ReadDirectoryChangesW`;
   - Atualiza a janela mapeada e anexa os novos offsets de linha ao índice;
   - Realiza rolagem automática suave para manter a última linha do documento sempre visível.
4. Para pausar a rolagem automática e inspecionar linhas anteriores, basta rolar a visualização para cima ou pressionar **Ctrl+L** novamente.

---

### 5. Motor de Busca Vetorial e Filtragem de Logs

A barra de busca rápida do Munin App foi projetada para pesquisa interativa e instantânea em bases massivas.

#### Como pesquisar:
1. Pressione **Ctrl+F** para exibir a barra de busca no topo do visualizador.
2. Digite o termo desejado no campo de texto.
3. Configure os modificadores de pesquisa conforme a necessidade:
   - **Diferenciar Maiúsculas/Minúsculas (`Aa`)**: ativa correspondência exata de caixa alta/baixa.
   - **Palavra Inteira (`\b`)**: restringe a busca a palavras completas delimitadas por espaços ou pontuação.
   - **Expressão Regular (`.*`)**: habilita busca por regex completa (sintaxe PCRE compatível).
4. Pressione **Enter** ou clique em **Próximo** / **Anterior** (`F3` / `Shift+F3`) para saltar diretamente entre as ocorrências encontradas.
5. O Minimapa na lateral direita destacará automaticamente todas as linhas correspondentes em toda a extensão do documento.

#### Modo de Filtragem:
- Ao ativar a opção de filtro na barra de busca, o visualizador principal oculta temporariamente as linhas que não atendem ao critério, exibindo uma lista compactada exclusivamente com os registros correspondentes.
- Um badge informativo na barra de ferramentas indicará o total de linhas filtradas (ex: `Exibindo 1.250 de 15.000.000 linhas`).
- Pressione **Esc** ou clique no botão **Limpar Filtro** para restaurar a visualização completa do arquivo.

---

### 6. Linha do Tempo e Análise Temporal (Time-Slice)

Muitos logs de produção possuem carimbos de data/hora (timestamps ISO-8601, padrões de servidor web, syslog, etc.). O painel **Time-Slice** analisa esses carimbos para prover uma visão analítica de densidade temporal.

#### Como utilizar:
1. Pressione **Ctrl+T** ou clique em **Timeline** na barra de ferramentas.
2. O painel inferior exibirá um histograma de barras com o volume de logs distribuído ao longo das horas e minutos do arquivo.
3. Cores diferenciadas indicam a densidade de eventos por período:
   - Barras padrão para tráfego normal de mensagens informativas;
   - Picos em vermelho indicando concentração anormal de erros ou exceções (*error bursts*).
4. Clique sobre qualquer barra do histograma para que o visualizador principal salte instantaneamente para o primeiro registro daquele intervalo de tempo.

---

### 7. Inspetor de Linhas e JSON Estruturado

Logs modernos gerados por microsserviços e bibliotecas estruturadas frequentemente gravam payloads complexos em formato JSON em cada linha.

#### Como utilizar:
1. Pressione **Ctrl+J** ou clique em **JSON** na barra de ferramentas para exibir o painel lateral do Inspetor JSON.
2. Clique sobre qualquer linha da tabela principal.
3. O Inspetor detectará automaticamente se a linha contém dados estruturados:
   - Se for um objeto JSON válido, ele será dissecado em uma **árvore hierárquica navegável** com nós expansíveis para chaves, arrays e valores;
   - As propriedades de nível crítico (como `level`, `status`, `error`, `timestamp`, `traceId`) recebem realce visual diferenciado;
   - O painel disponibiliza um botão rápido para copiar o payload formatado e identado (*pretty-printed*) para a área de transferência do Windows.

---

### 8. Minimapa e Barra de Rolagem Visual

A barra de rolagem vertical do Munin App não é um controle estático, mas sim um **Minimapa de Densidade**:
- **Mapa de Calor**: Projeta uma representação gráfica proporcional do arquivo inteiro ao longo da altura da janela;
- **Marcadores Cromáticos**:
  - `Azul / Ciano`: Resultados da pesquisa ativa;
  - `Vermelho`: Linhas contendo níveis críticos de severidade (`ERROR`, `FATAL`, `CRITICAL`);
  - `Amarelo`: Linhas de aviso (`WARN`, `WARNING`);
- **Navegação Rápida**: Clicar em qualquer ponto do minimapa reposiciona a tabela diretamente na região temporal ou estrutural correspondente.

---

### 9. Gerador de Logs Sintéticos para Testes e Estresse

Para fins de benchmark, demonstrações ou validação de scripts de terceiros, o Munin App inclui um utilitário embutido de geração sintética de logs.

#### Como gerar arquivos de teste:
1. Pressione **Ctrl+G** ou acesse **Arquivo > Gerador de Logs...**.
2. Selecione o tamanho desejado do arquivo sintético:
   - `100 MB` (~800.000 linhas);
   - `500 MB` (~4.000.000 linhas);
   - `1 GB` (~8.000.000 linhas).
3. Escolha o diretório de destino e confirme a geração.
4. O gerador produzirá registros realistas contendo timestamps sincronizados, níveis de severidade ponderados (`INFO` 70%, `DEBUG` 15%, `WARN` 10%, `ERROR` 5%) e payloads JSON estruturados para validação imediata no próprio Munin App.

---

### 10. Temas, Barra de Título Imersiva e Customização Visual

O Munin App adapta-se nativamente à identidade visual do Windows 11:
- **Adaptação Automática ao Tema do Sistema**:
  - Quando o usuário altera o tema do Windows 11 entre Claro e Escuro, o Munin App intercepta o evento do sistema operacional e atualiza a paleta de cores de todos os componentes da interface em tempo real;
  - Cores de fundo, bordas, textos, árvores de JSON e tabelas são ajustadas dinamicamente para garantir contraste confortável e evitar fadiga visual.
- **Barra de Título Imersiva DWM**:
  - Utiliza chamadas da API nativa `DwmSetWindowAttribute` (`DWMWA_USE_IMMERSIVE_DARK_MODE` e atributos de Mica) para que a barra de título da janela acompanhe a cor e o estilo visual do sistema sem cortes ou bordas brancas indesejadas.

---

### 11. Internacionalização em Tempo de Execução

O Munin App foi desenvolvido com suporte completo a múltiplos idiomas:
- **Idiomas Suportados**:
  - Português (Brasil) — `pt_BR`;
  - English (United States) — `en_US`.
- **Como Alternar o Idioma**:
  - Acesse o menu **Configurações > Idioma** e selecione o idioma desejado.
  - Toda a interface (menus, barra de status, botões, tooltips informativos, diálogos de ajuda e mensagens do sistema) é traduzida instantaneamente em tempo de execução sem requerer reinicialização do executável.

---

### 12. Gerenciamento de Janelas e Diálogos Modeless

Para garantir máxima produtividade em fluxos de auditoria e depuração técnica:
- Os diálogos **Sobre** (`Opções > Sobre`) e **Manual de Instruções** (`F1`) são implementados como janelas **modeless (não-modais)** desacopladas perante o gerenciador de janelas do sistema operacional.
- O usuário pode manter o Manual ou o diálogo Sobre abertos em uma tela secundária enquanto interage, pesquisa e navega livremente pelo visualizador de logs na janela principal.
- A ordem de sobreposição no eixo Z (*Z-Order*) é respeitada: clicar sobre a janela principal traz a interface principal para frente, sem que os diálogos fiquem travados no topo forçado.

---

### 13. Tabela Completa de Atalhos de Teclado

| Atalho | Contexto | Descrição da Ação |
| :--- | :--- | :--- |
| `Ctrl+O` | Geral | Abre o diálogo para selecionar e abrir um arquivo de log |
| `Ctrl+F` | Geral | Exibe ou oculta a barra de busca rápida |
| `Ctrl+T` | Geral | Exibe ou oculta a linha do tempo / Time-Slice |
| `Ctrl+J` | Geral | Exibe ou oculta o painel do Inspetor JSON |
| `Ctrl+L` | Geral | Ativa ou desativa o monitoramento ao vivo (Live Tail) |
| `Ctrl+G` | Geral | Abre o gerador de arquivos de log de teste |
| `F3` | Busca | Salta para a próxima ocorrência encontrada |
| `Shift+F3` | Busca | Salta para a ocorrência anterior encontrada |
| `Esc` | Geral | Limpa o filtro de busca ativo ou fecha painéis auxiliares |
| `F1` | Geral | Abre o Manual de Instruções da aplicação |
| `Home` | Visualizador | Salta para a primeira linha do documento |
| `End` | Visualizador | Salta para a última linha do documento |
| `Page Up` | Visualizador | Rola uma página para cima |
| `Page Down` | Visualizador | Rola uma página para baixo |
| `Up` / `Down` | Visualizador | Navega linha por linha |
| `Ctrl+C` | Visualizador / JSON | Copia o registro ou nó selecionado para a área de transferência |
| `Alt+A` | Menus | Abre o menu "Arquivo" |
| `Alt+E` | Menus | Abre o menu "Exibir" |
| `Alt+C` | Menus | Abre o menu "Configurações" |
| `Alt+B` ou `Alt+S` | Menus | Abre o menu "Sobre" |

---

### 14. Diagnóstico, Desempenho e Solução de Problemas

#### Consumo de Memória RAM:
- Devido à arquitetura baseada em arquivos mapeados em memória, o consumo de memória RAM do processo `Munin.exe` permanece geralmente entre 40 MB e 90 MB, independentemente de o arquivo aberto ter 100 MB ou 20 GB.
- O Windows gerencia o cache de disco conforme a memória disponível, sem risco de saturação da memória física do usuário.

#### Arquivos Abertos em Uso por Outros Processos:
- O Munin App abre arquivos com as flags Win32 `FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE`. Isso significa que servidores e daemons externos podem continuar gravando no log normalmente enquanto o arquivo está sendo inspecionado no Munin App.

#### Suíte de Benchmark (`MuninBench.exe`):
- O projeto disponibiliza o executável auxiliar `MuninBench` para teste de estresse da engine:
  - Mede a taxa de transferência de I/O puro e o rendimento da vetorização SIMD AVX2 em GB/s;
  - Testa o tempo de resposta da busca paralela multithread sob diferentes padrões de regex.

---

### 15. Utilitário de Linha de Comando (`munin-app-cli`)

Para fluxos de trabalho automatizados, scripts DevOps, integração contínua (CI/CD) e administradores de sistemas que preferem o terminal, o Munin App inclui o utilitário nativo de linha de comando `munin-app-cli.exe`.

O executável da CLI é 100% autônomo, não inicializa a interface gráfica do Qt e compartilha diretamente a engine `MuninEngine` em C++20 com aceleração SIMD AVX2/SSE4.2 e arquivos mapeados em memória Win32.

#### Acesso Global no Windows (AppExecutionAlias)
Quando o Munin App é instalado via pacote MSIX ou pelos instaladores oficiais, o alias `munin-app-cli.exe` é automaticamente registrado nas variáveis de ambiente do sistema. Isso permite abrir o PowerShell, Prompt de Comando (CMD) ou Windows Terminal e executar o comando de qualquer diretório:
```cmd
munin-app-cli --version
```

#### Comandos Disponíveis

##### 1. Extração de Metadados e Métricas (`info`)
Analisa a estrutura do log, constrói a tabela de índices em memória e exibe a velocidade de processamento:
```cmd
munin-app-cli.exe info "C:\Logs\producao.log"
```
**Exemplo de Saída:**
```text
File: C:\Logs\producao.log
Size: 4250.00 MB (4456448000 bytes)
Total Lines: 38542100
Chunks: 38543 (1000 lines/chunk)
Indexing Time: 1.2840 s
Throughput: 3310.25 MB/s
```

##### 2. Busca Paralela de Alta Velocidade (`search`)
Pesquisa padrões textuais ou expressões regulares utilizando todos os núcleos do processador:
```cmd
munin-app-cli.exe search "C:\Logs\producao.log" "TimeoutException" --ignore-case --max 20
```

**Parâmetros suportados:**
- `--ignore-case`, `-i`: Busca insensível a maiúsculas e minúsculas.
- `--regex`, `-r`: Interpreta o termo de pesquisa como uma Expressão Regular.
- `--invert`, `-v`: Exibe apenas as linhas que **não** contêm o padrão pesquisado.
- `--max <n>`: Define o limite máximo de ocorrências exibidas (padrão: 1000).
- `--format <text|json>`: Define o formato de saída (legível ou JSON estruturado para pipelines).
- `--no-banner`: Suprime o cabeçalho inicial para integração limpa em scripts.

**Exemplo de Saída em JSON para automação:**
```cmd
munin-app-cli.exe search "C:\Logs\app.log" "FATAL" --format json --no-banner
```

##### 3. Monitoramento em Tempo Real (`tail`)
Acompanha gravações ativas no arquivo de log exibindo as últimas entradas e transmitindo instantaneamente novos registros gravados:
```cmd
munin-app-cli.exe tail "C:\Logs\servidor_web.log"
```
- Exibe inicialmente as últimas 10 linhas do log;
- Monitora novos acréscimos via eventos do sistema operacional sem consumo excessivo de CPU;
- Detecta automaticamente quando o arquivo é rotacionado ou truncado pelo servidor;
- Pressione `Ctrl+C` para encerrar o monitoramento.

##### 4. Testes de Desempenho e Diagnóstico (`bench`)
Avalia a capacidade de processamento do hardware local:
```cmd
munin-app-cli.exe bench
```
Caso não seja informado um arquivo, o utilitário afere as taxas de transferência de mapeamento de memória, escaneamento vetorial e busca multithread gerando uma amostra sintética de teste.

</details>

---

## <a id="enus"></a>English (United States)

> This manual provides complete documentation of the operational workflows, user interface components, low-level architecture, and configuration of **Munin App — Massive Log Reader**.

<details>
<summary>Click to expand / collapse the MANUAL in English</summary>

### 1. Overview and Architectural Genesis

**Munin App** is an ultra-high-performance log viewer and analytical workbench built natively in **C++20** with a responsive **Qt 6** interface for the Windows 11 platform. It was engineered to overcome the classic bottleneck where traditional text editors and log utilities freeze or exhaust physical RAM when opening archives ranging from hundreds of megabytes to tens of gigabytes.

#### Core Architectural Pillars:
1. **Win32 Memory-Mapped Files (Zero-Copy Paging)**:
   - Instead of reading the entire file into application heap memory, Munin App maps file data into virtual memory pages using native Windows NT kernel primitives (`CreateFileMappingW` and `MapViewOfFile`).
   - The OS kernel manages memory paging on demand directly from the disk cache, allowing 1 GB, 10 GB, or larger files to open in milliseconds with minimal physical memory footprint.
2. **SIMD Vectorized Indexing (AVX2 & SSE4.2)**:
   - Line delimiter scanning (`\n`) is vectorized using 256-bit AVX2 instructions (`_mm256_movemask_epi8`) and 128-bit SSE4.2 instructions (`_mm_movemask_epi8`), with an optimized scalar fallback.
   - Line index scanning throughput routinely surpasses 3 GB/s.
3. **Sparse Chunk Indexing (`ChunkIndex`)**:
   - The file is indexed into 64 KB sparse chunks, providing constant-time $O(1)$ random seek to any record across millions of lines.
4. **Asynchronous Kernel Live Tail (`LiveTailWatcher`)**:
   - Background file append monitoring powered by the native Win32 `ReadDirectoryChangesW` kernel API without polling loops.
5. **Multi-Core Parallel Search (`ParallelSearch`)**:
   - Search workloads are divided into pre-allocated chunk intervals across all available logical CPU cores, supporting JIT Regular Expressions, literal search, and matched-line filter modes.
6. **Strict Privacy and Sovereignty**:
   - 100% offline and autonomous operation. No logs, terms, file paths, or metrics are ever shared or transmitted over networks.

---

### 2. Main Window Interface and Layout

Munin App's workspace is structured for maximum visibility, high contrast, and immediate response:

```text
+-----------------------------------------------------------------------------------+
|  File       View      Settings         About                         [ _ ] [ 口 ] [ X ]|
+-----------------------------------------------------------------------------------+
| [ Open ] [ Live Tail ] [ Search ] [ Timeline ] [ JSON ] [ Generator ]| Filter: Active|
+-----------------------------------------------------------------------------------+
| [ Quick Search: [Text or Regex...]   (Aa) (Word)    (.*)  [< Prev] [Next >] (X) ] |
+----------------------------------------------------------+------------------------+
| Line  | Log Record                                       |       DENSITY          |
|-------+--------------------------------------------------|       MINIMAP          |
| 1     | 2026-10-14 10:00:01.123 [INFO] Server initialized|                        |
| 2     | 2026-10-14 10:00:02.456 [DEBUG] Connection accept|       [   ]            |
| 3     | 2026-10-14 10:00:05.789 [WARN] High latency spike|       [ # ] -> Error   |
| 4     | 2026-10-14 10:00:10.012 [ERROR] Connection failed|       [ * ] -> Match   |
| ...   | ...                                              |                        |
+----------------------------------------------------------+------------------------+
| [ Time-Slice Histogram (Event Frequency by Hour / Minute) ]                       |
+-----------------------------------------------------------------------------------+
| [ Structured JSON Inspector: Hierarchical Property Tree ]                         |
+-----------------------------------------------------------------------------------+
| File: app.log | Size: 4.25 GB | Lines: 38,542,100 | Speed: 3.42 GB/s              |
+-----------------------------------------------------------------------------------+
```

#### UI Components:
- **Fluent Sticky Menu Bar (`StickyMenu`)**: Quick-access menus navigable via keyboard shortcuts (`Alt+A`, `Alt+E`, `Alt+C`, `Alt+B`).
- **Main ToolBar**: Quick buttons to open files, toggle Live Tail, trigger search, show the temporal timeline, open the structured JSON inspector, and launch the synthetic log generator.
- **Virtualized Log Table (`LogView`)**: High-performance custom table view based on `QAbstractItemView` and `QTableView`, only rendering visible rows inside the viewport.
- **Density Minimap Scrollbar (`MinimapScrollBar`)**: Custom vertical scrollbar providing an overall heatmap of log density and highlighted color markers for search hits and error levels.
- **Status Bar**: Real-time operational diagnostics:
  - Current file name and path;
  - Formatted file size;
  - Total and filtered line counts;
  - Indexing and processing speed (GB/s);
  - Process RAM consumption;
  - Live Tail monitoring status (`Active` or `Paused`).

---

### 3. File Opening and Navigation

#### Methods to open a log file:
1. Menu: **File > Open File...** (`Ctrl+O`).
2. ToolBar: Click the **Open** button.
3. **Drag & Drop**: Drag a log file of any size from Windows Explorer directly into the Munin App window.

#### Navigation controls:
- `Home` / `End`: Jump to the top or bottom of the document.
- `Page Up` / `Page Down`: Scroll up or down by one screen height.
- `Up` / `Down` arrows: Line-by-line navigation.
- Smooth mouse wheel scrolling with zero frame drops.

---

### 4. Real-Time Monitoring (Live Tail)

The **Live Tail** feature allows developers and system administrators to monitor actively written server and application logs in real time.

#### How to use:
1. Press **Ctrl+L** or click the **Live Tail** button on the toolbar.
2. The button highlights to indicate active append tracking.
3. As external processes write new lines to disk, Munin App:
   - Receives kernel file modification notifications via `ReadDirectoryChangesW`;
   - Appends new line offsets to the chunk index;
   - Smoothly auto-scrolls the table view to keep the latest entries visible.
4. Scrolling up pauses auto-scrolling to let you inspect earlier logs without interruption. Pressing **Ctrl+L** again resumes tracking.

---

### 5. Parallel Search Engine and Filtering

#### Quick Search:
1. Press **Ctrl+F** to reveal the floating search bar.
2. Enter the search string or pattern.
3. Configure search flags:
   - **Case Sensitive (`Aa`)**: Match exact letter cases.
   - **Whole Word (`\b`)**: Restrict search to standalone tokens.
   - **Regular Expression (`.*`)**: Enable full PCRE-compatible regex searching.
4. Press **Enter** or click **Next** / **Previous** (`F3` / `Shift+F3`) to cycle through matching records.
5. Search matches are highlighted both in the log table and across the vertical Minimap.

#### Filter Mode:
- Activate the filter option to condense the table view, displaying solely rows that match the query criteria.
- Press **Esc** or click the **Clear Filter** button to restore full log visibility.

---

### 6. Time Distribution Timeline (Time-Slice)

For logs containing timestamps (ISO-8601, syslog, web server formats), the **Time-Slice** panel aggregates activity density over time.

#### How to use:
1. Press **Ctrl+T** or click **Timeline** on the toolbar.
2. An interactive bar histogram illustrates log volume per time interval.
3. Error bursts are highlighted with distinct red bars to signal operational incidents immediately.
4. Clicking any bar in the histogram seeks the log view directly to that timeframe.

---

### 7. Structured JSON & Record Inspector

#### How to use:
1. Press **Ctrl+J** or click **JSON** on the toolbar.
2. Click any row containing JSON structured data in the main table.
3. The inspector automatically detects the JSON structure, presenting it as an interactive, syntax-highlighted property tree.
4. Click the copy button to copy the formatted, indented JSON payload to the Windows clipboard.

---

### 8. Minimap Density Scrollbar

The custom vertical scrollbar renders an analytical overview of the entire file:
- Visual density gradient representing log activity throughout the file;
- `Cyan/Blue markers`: Active search matches;
- `Red markers`: Errors and fatal exceptions (`ERROR`, `FATAL`, `CRITICAL`);
- `Yellow markers`: Warnings (`WARN`, `WARNING`);
- Clicking anywhere on the minimap jumps directly to that relative position in the log.

---

### 9. Synthetic Log Generator for Benchmarking

#### How to use:
1. Press **Ctrl+G** or select **File > Synthetic Log Generator...**.
2. Select the target file size: `100 MB`, `500 MB`, or `1 GB`.
3. Choose a destination folder and click generate.
4. Munin App creates realistic logs with balanced severity levels (`INFO`, `DEBUG`, `WARN`, `ERROR`), synchronized timestamps, and JSON payloads for immediate stress testing.

---

### 10. Themes & Windows 11 Visual Adaptation

- **Automatic Theme Switching**: Senses Windows 11 Dark and Light mode transitions in real time, recalculating UI palettes without requiring an application restart.
- **DWM Mica Integration**: Native Windows Desktop Window Manager integration for dark title bars and seamless frame styling.

---

### 11. Dynamic Runtime Retranslation

- **Supported Languages**: English (`en_US`) and Portuguese (`pt_BR`).
- Switch dynamically in **Settings > Language** with zero downtime or reload required.

---

### 12. Modeless Dialogs and Z-Order Window Management

- The **About** and **Instruction Manual** dialogs are top-level modeless windows.
- Users can switch focus freely between dialogs and the main log viewer, positioning them across multi-monitor setups without modality locks.

---

### 13. Complete Keyboard Shortcuts Reference

| Shortcut | Context | Action |
| :--- | :--- | :--- |
| `Ctrl+O` | Global | Open log file |
| `Ctrl+F` | Global | Toggle quick search bar |
| `Ctrl+T` | Global | Toggle Time-Slice distribution timeline |
| `Ctrl+J` | Global | Toggle structured JSON inspector |
| `Ctrl+L` | Global | Toggle Live Tail append monitoring |
| `Ctrl+G` | Global | Open synthetic log generator |
| `F3` / `Shift+F3` | Search | Jump to next / previous match |
| `Esc` | Global | Clear filter or dismiss search bar |
| `F1` | Global | Open the Instruction Manual |
| `Home` / `End` | Log View | Scroll to start / end of file |
| `Page Up` / `Page Down` | Log View | Scroll one page up / down |
| `Ctrl+C` | Log View / JSON | Copy selected row or JSON payload |
| `Alt+A` | Menus | Open "File" menu |
| `Alt+E` | Menus | Open "View" menu |
| `Alt+C` | Menus | Open "Settings" menu |
| `Alt+B` or `Alt+S` | Menus | Open "About" menu |

---

### 14. Performance, Diagnostics and Troubleshooting

- **Low RAM Footprint**: Virtualized memory mapping ensures RAM usage remains consistently low (~40 MB to ~90 MB) regardless of whether you inspect a 100 MB or 20 GB file.
- **Concurrent File Sharing**: Files are opened with non-exclusive share modes (`FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE`), allowing server processes to continue appending logs unhindered.
- **Benchmarking Suite**: Run `MuninBench.exe` from the command line to stress-test your storage subsystem, AVX2 SIMD throughput, and multi-threaded search scaling.

---

### 15. Command-Line Interface (`munin-app-cli`)

For automated workflows, DevOps pipelines, CI/CD scripts, and system administrators who operate from the terminal, Munin App provides the native `munin-app-cli.exe` command-line utility.

The CLI executable is completely standalone, does not initialize the Qt GUI stack, and directly shares the high-performance C++20 `MuninEngine` with AVX2/SSE4.2 SIMD acceleration and Win32 memory-mapped files.

#### Global Windows Terminal Access (AppExecutionAlias)
When Munin App is installed via the official MSIX package or installer, the `munin-app-cli.exe` execution alias is registered with the Windows subsystem. You can open Windows Terminal, PowerShell, or Command Prompt (CMD) from any directory and execute:
```cmd
munin-app-cli --version
```

#### Available Commands

##### 1. Log Metadata and Metric Extraction (`info`)
Inspects the log file structure, generates line chunk indexes, and reports throughput:
```cmd
munin-app-cli.exe info "C:\Logs\production.log"
```
**Sample Output:**
```text
File: C:\Logs\production.log
Size: 4250.00 MB (4456448000 bytes)
Total Lines: 38542100
Chunks: 38543 (1000 lines/chunk)
Indexing Time: 1.2840 s
Throughput: 3310.25 MB/s
```

##### 2. High-Speed Multi-Core Parallel Search (`search`)
Searches textual patterns or regular expressions across all logical CPU cores:
```cmd
munin-app-cli.exe search "C:\Logs\production.log" "TimeoutException" --ignore-case --max 20
```

**Supported Options:**
- `--ignore-case`, `-i`: Case-insensitive search.
- `--regex`, `-r`: Treat query as a regular expression.
- `--invert`, `-v`: Invert match (show lines that do not match).
- `--max <n>`: Limit maximum reported matches (default: 1000).
- `--format <text|json>`: Select output format (plain text or structured JSON for piping).
- `--no-banner`: Suppress header banner for clean script output.

**Sample Structured JSON Output:**
```cmd
munin-app-cli.exe search "C:\Logs\app.log" "FATAL" --format json --no-banner
```

##### 3. Real-Time Log Streaming (`tail`)
Streams new entries appended to live log files:
```cmd
munin-app-cli.exe tail "C:\Logs\webserver.log"
```
- Displays the last 10 lines upon launch;
- Streams new lines using zero-overhead Windows kernel notifications without polling;
- Automatically handles log rotation and file truncation;
- Press `Ctrl+C` to terminate the stream.

##### 4. Hardware Benchmark Suite (`bench`)
Benchmarks storage bandwidth, SIMD vector scanning, and parallel search scaling:
```cmd
munin-app-cli.exe bench
```
If no file argument is provided, a synthetic workload is generated and benchmarked automatically.

</details>