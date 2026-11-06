# Guia do Usuário / Operations User Guide — Munin App

<p align="center">
  <b>Selecione o idioma / Select language:</b><br>
  <a href="#ptbr">🇧🇷 Português (BR)</a> | <a href="#enus">🇺🇸 English (US)</a>
</p>

---

## <a id="ptbr"></a>🇧🇷 Português (BR)

<details>
<summary>Clique para expandir / recolher o guia do usuário em português</summary><br>

# Munin App — Manual de Operação e Guia do Usuário

**Versão:** `2026.11.6.0`  
**Data:** `6 de novembro de 2026`  
**Plataforma:** Windows 11 / Windows 10 (x64)  
**Autor:** Fernando Nillsson Cidade  

---

## 1. Primeiros Passos e Abertura de Arquivos

O **Munin App** foi projetado para abrir arquivos de log massivos (de centenas de megabytes a múltiplos gigabytes) instantaneamente:

### 1.1. Formas de Abrir Arquivos
1. **Menu Principal ou Atalho:** Pressione `Ctrl+O` ou clique no botão **Abrir Arquivo** na barra de ferramentas.
2. **Arrastar e Soltar (Drag & Drop):** Arraste qualquer arquivo de log de uma janela do Windows Explorer diretamente para a janela do Munin App.
3. **Linha de Comando:** Passe o caminho do arquivo como argumento posicional:
   ```powershell
   Munin.exe "D:\Logs\production_backend.log"
   ```
4. **Caminhos de Rede UNC:** Suporte nativo a caminhos compartilhados como `\\servidor\logs\app.log`.

---

## 2. Recursos e Fluxos de Análise

### 2.1. Navegação Virtualizada e Minimapa de Densidade
- **Tabela Virtual:** O modelo em memória renderiza apenas as linhas que cabem na janela atual, permitindo rolagem contínua a 60 FPS com consumo mínimo de memória RAM.
- **Minimapa Lateral:** Barra de rolagem visual integrada que plota a densidade de eventos ao longo do documento:
  - **Marcadores Vermelhos:** Erros críticos e exceções.
  - **Marcadores Amarelos:** Alertas operacionais e advertências.
  - **Marcadores Ciano/Azul:** Ocorrências encontradas na pesquisa atual.
- **Clique Direto no Minimapa:** Salte instantaneamente para qualquer região do arquivo com um único clique.

### 2.2. Monitoramento em Tempo Real (Live Tail)
- Ative o modo **Live Tail** pressionando `Ctrl+L` ou clicando no botão correspondente na barra de ferramentas.
- O aplicativo utiliza notificações de kernel do Windows (`ReadDirectoryChangesW`) para indexar e exibir novos dados anexados em tempo real sem loops de polling.
- **Auto-Scroll Inteligente:** O aplicativo acompanha automaticamente o final do arquivo. Se você rolar para cima para investigar uma linha anterior, o auto-scroll pausa de forma transparente para não interromper sua leitura.

### 2.3. Motor de Busca Paralela e Modo Filtro
- Pressione `Ctrl+F` para alternar a barra de busca.
- **Opções de Busca:**
  - **Diferenciar Maiúsculas/Minúsculas (`Aa`):** Correspondência exata de caixa.
  - **Palavra Inteira (`\b`):** Ignora ocorrências que façam parte de outras palavras.
  - **Expressões Regulares (`.*`):** Ativa o motor PCRE2 JIT de alta performance.
- **Modo Filtrar Linhas:** Oculte todas as linhas não correspondentes e visualize exclusivamente as ocorrências encontradas em sequência contínua.

### 2.4. Histograma Temporal (Time-Slice Scanner)
- Pressione `Ctrl+T` para exibir a linha do tempo.
- O Munin App analisa automaticamente os timestamps no padrão ISO-8601 e plota um histograma com a densidade de registros por intervalo.
- **Isolamento de Incidentes:** Clique sobre qualquer barra do gráfico para filtrar apenas os eventos ocorridos naquela janela temporal específica.

### 2.5. Inspetor Estruturado de Linhas e JSON
- Pressione `Ctrl+J` para abrir o painel retrátil de inspeção.
- Ao clicar em qualquer linha contendo payload JSON, o painel formata a estrutura em uma árvore navegável com realce sintático.
- **Cópia Rápida:** Clique com o botão direito para copiar chaves ou valores isolados para a área de transferência.

### 2.6. Gerador Sintético de Logs para Testes
- Acesse **Ferramentas > Gerador de Logs de Teste**.
- Gere conjuntos de dados padronizados (100 MB, 500 MB ou 1 GB) localmente para aferir taxas de transferência e validar a performance da máquina.

---

## 3. Tabela Completa de Atalhos de Teclado

| Atalho | Ação Correspondente |
| :--- | :--- |
| `Ctrl+O` | Abrir arquivo de log |
| `Ctrl+F` | Alternar barra de busca rápida |
| `Ctrl+L` | Alternar monitoramento em tempo real (Live Tail) |
| `Ctrl+T` | Alternar histograma temporal (Time-Slice) |
| `Ctrl+J` | Alternar painel de inspeção de JSON estruturado |
| `Ctrl++` / `Ctrl+-` | Aumentar / diminuir tamanho da fonte |
| `Ctrl+0` | Restaurar tamanho padrão da fonte |
| `F1` | Abrir Manual de Instruções modeless |
| `F3` / `Shift+F3` | Próxima / anterior ocorrência da busca |
| `F5` | Recarregar arquivo do disco |
| `Ctrl+Home` / `Ctrl+End` | Saltar para a primeira / última linha |

</details>

---

## <a id="enus"></a>🇺🇸 English (US)

<details>
<summary>Click to expand / collapse user guide in English</summary><br>

# Munin App — Operations User Guide & Manual

**Version:** `2026.11.6.0`  
**Date:** `November 6, 2026`  
**Target Platform:** Windows 11 / Windows 10 (x64)  
**Author:** Fernando Nillsson Cidade  

---

## 1. Getting Started & Opening Files

**Munin App** opens multi-gigabyte log files instantaneously:

### 1.1. Ways to Open Files
1. **Menu or Keyboard Shortcut:** Press `Ctrl+O` or click **Open File** on the toolbar.
2. **Drag & Drop:** Drag any log file from Windows Explorer into the Munin App window.
3. **Command Line:** Supply file path as a positional argument:
   ```powershell
   Munin.exe "D:\Logs\production_backend.log"
   ```
4. **Network UNC Paths:** Full support for network shares such as `\\server\share\app.log`.

---

## 2. Core Features & Analytical Workflows

### 2.1. Virtualized Navigation & Minimap Density Scrollbar
- **Virtual Table:** The in-memory model materializes only rows currently visible in the window, keeping memory low (< 60 MB) and scrolling smooth at 60 FPS.
- **Minimap Scrollbar:** Visual density heatmap showing document distribution:
  - **Red markers:** Critical errors and exceptions.
  - **Yellow markers:** Warnings.
  - **Cyan/Blue markers:** Search hits.
- **Direct Navigation:** Click anywhere on the minimap to jump instantly to that location.

### 2.2. Real-Time Live Tail Monitoring
- Toggle **Live Tail** by pressing `Ctrl+L` or clicking the toolbar button.
- Uses native Windows `ReadDirectoryChangesW` kernel events to index newly appended records without polling loops.
- **Smart Auto-Scroll:** Automatically scrolls down as new lines arrive. Pauses automatically when user scrolls up to review earlier lines.

### 2.3. Parallel Search & Filter Mode
- Press `Ctrl+F` to toggle the search bar.
- **Search Options:**
  - **Match Case (`Aa`):** Exact case matching.
  - **Whole Word (`\b`):** Ignore partial word matches.
  - **Regular Expressions (`.*`):** JIT-compiled PCRE2 regex engine.
- **Filter Lines Mode:** Hide non-matching rows and display only matched results in a continuous sequence.

### 2.4. Time-Slice Histogram
- Press `Ctrl+T` to toggle the timeline histogram.
- Automatically parses ISO-8601 timestamps and displays event density over time.
- **Incident Isolation:** Click any histogram bar to isolate and view only events within that time window.

### 2.5. Structured JSON Inspector
- Press `Ctrl+J` to toggle the collapsible inspector panel.
- Clicking any row with JSON payloads formats the structure into a syntax-highlighted tree.
- **Quick Copy:** Right-click to copy keys or values to clipboard.

### 2.6. Synthetic Log Generator
- Open **Tools > Test Log Generator**.
- Generate synthetic standardized files (100 MB, 500 MB, 1 GB) locally to measure throughput.

---

## 3. Keyboard Shortcuts Reference

| Shortcut | Action |
| :--- | :--- |
| `Ctrl+O` | Open log file |
| `Ctrl+F` | Toggle search bar |
| `Ctrl+L` | Toggle real-time Live Tail |
| `Ctrl+T` | Toggle time-slice histogram |
| `Ctrl+J` | Toggle JSON inspector |
| `Ctrl++` / `Ctrl+-` | Zoom in / zoom out font size |
| `Ctrl+0` | Reset font size |
| `F1` | Open Modeless Instruction Manual |
| `F3` / `Shift+F3` | Find next / previous hit |
| `F5` | Reload file from disk |
| `Ctrl+Home` / `Ctrl+End` | Jump to first / last line |

</details>
