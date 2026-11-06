# Referência da Linha de Comando / CLI Reference — Munin App

<p align="center">
  <b>Selecione o idioma / Select language:</b><br>
  <a href="#ptbr">🇧🇷 Português (BR)</a> | <a href="#enus">🇺🇸 English (US)</a>
</p>

---

## <a id="ptbr"></a>🇧🇷 Português (BR)

<details>
<summary>Clique para expandir / recolher a referência da linha de comando em português</summary><br>

# Munin App — Referência da Linha de Comando e Ferramentas de Console

**Versão:** `2026.11.6.0`  
**Data:** `6 de novembro de 2026`  
**Executáveis:** `Munin.exe` (Aplicação Principal Desktop) | `munin-app-cli.exe` (Utilitário Headless de Linha de Comando) | `MuninBench.exe` (Utilitário de Benchmark e Diagnóstico) | `find_VersionEditor.exe` (Auditor de Versões)  

---

## 1. Visão Geral

O ecossistema **Munin App** fornece suporte a comandos via terminal PowerShell, CMD e Windows Terminal, permitindo associar arquivos de log ao aplicativo, invocar a abertura automática em rotinas de suporte e executar aferições de desempenho da engine C++20.

---

## 2. Aplicação Desktop (`Munin.exe`)

O executável gráfico aceita o caminho de um arquivo de log como primeiro argumento posicional. Ao ser informado, o Munin App inicializa a interface gráfica e realiza imediatamente o mapeamento virtual de memória do arquivo:

```cmd
Munin.exe [caminho_do_arquivo]
```

### Exemplos de Uso:

```powershell
# Abrir um arquivo de log local imediatamente
Munin.exe "C:\inetpub\logs\LogFiles\W3SVC1\u_ex261017.log"

# Abrir um arquivo massivo em caminho de rede UNC
Munin.exe "\\servidor-arquivos\producao\logs\cluster_app.log"

# Utilizar o alias de execução do Windows Terminal (via pacote Microsoft Store / MSIX)
munin "D:\Logs\production_backend_10gb.log"
```

---

## 3. Utilitário Standalone de Linha de Comando (`munin-app-cli.exe`)

O executável `munin-app-cli.exe` é compilado a partir de `source/cli/main_cli.cpp` e opera em modo headless nativo de console (sem dependência de GUI ou inicialização do Qt Widgets), ideal para automação, pipelines de CI/CD, scripts em PowerShell/Bash e administração remota:

```cmd
munin-app-cli.exe <comando> [argumentos...] [opções...]
```

### Comandos Disponíveis:

| Comando | Descrição | Sintaxe / Opções |
| :--- | :--- | :--- |
| `search` | Executa busca de alta performance no arquivo | `munin-app-cli.exe search <arquivo> <termo> [--regex] [-i] [--limit <N>] [--format text\|json\|csv]` |
| `info` | Extrai metadados do arquivo (tamanho, linhas, SIMD e velocidade) | `munin-app-cli.exe info <arquivo> [--format text\|json]` |
| `tail` | Monitora continuamente a anexação de novos registros (estilo `tail -f`) | `munin-app-cli.exe tail <arquivo> [-n <linhas>] [-f] [--timeout <segundos>]` |
| `bench` | Executa bateria de aferição de vazão de leitura e busca paralela | `munin-app-cli.exe bench <arquivo>` |
| `--version` / `-v` | Exibe a versão oficial e informações de compilação | `munin-app-cli.exe --version` |
| `--help` / `-h` | Exibe a sintaxe completa e exemplos de execução | `munin-app-cli.exe --help` |

### Exemplos de Uso:

```powershell
# Inspecionar métricas e total de linhas de um arquivo de log
munin-app-cli.exe info "C:\Logs\production.log"

# Buscar termo com saída estruturada em JSON (até 50 ocorrências)
munin-app-cli.exe search "C:\Logs\app.log" "EXCEPTION" --limit 50 --format json

# Buscar utilizando expressão regular ignorando maiúsculas/minúsculas
munin-app-cli.exe search "C:\Logs\web.log" "HTTP/1\.[01]\" [45]\d\d" --regex -i

# Acompanhar gravação ao vivo exibindo as últimas 20 linhas
munin-app-cli.exe tail "C:\Logs\service.log" -n 20 -f
```

---

## 4. Utilitário de Benchmark da Engine (`MuninBench.exe`)

O executável `MuninBench.exe` é compilado a partir de `source/engine/TestEngine.cpp` e serve para homologar a performance da máquina local, testando taxas de transferência reais em GB/s:

```cmd
MuninBench.exe [opções]
```

### Testes Executados pelo Benchmark:
1. **Throughput de Mapeamento Win32:** Abertura e visualização de páginas em memória virtual.
2. **Taxa de Escaneamento SIMD:** Aferição de gigabytes processados por segundo com instruções AVX2 (256 bits) e SSE4.2 (128 bits).
3. **Escalonamento Multi-Core:** Eficiência da busca paralela com divisão por blocos de 64 KB entre todas as threads de hardware.

---

## 5. Utilitário de Verificação de Integridade (`find_VersionEditor.exe`)

Ferramenta desenvolvida para auditoria e sincronização contínua de versões e metadados nos 28 arquivos do repositório:

```powershell
# Verificar integridade e consistência de todos os 28 arquivos do projeto
.\build\build_Mocks_mingw\find_VersionEditor.exe --check .

# Executar a interface gráfica de sincronização de versão
.\build\build_Mocks_mingw\find_VersionEditor.exe
```

</details>

---

## <a id="enus"></a>🇺🇸 English (US)

<details>
<summary>Click to expand / collapse CLI reference in English</summary><br>

# Munin App — Command Line Reference & Console Utilities

**Version:** `2026.11.6.0`  
**Date:** `November 6, 2026`  
**Executables:** `Munin.exe` (Primary Desktop Application) | `munin-app-cli.exe` (Standalone Headless CLI Utility) | `MuninBench.exe` (Benchmark & Diagnostic Utility) | `find_VersionEditor.exe` (Version Auditor)  

---

## 1. Overview

The **Munin App** ecosystem supports execution via PowerShell, Command Prompt, and Windows Terminal, enabling automated file inspection and engine performance diagnostic runs.

---

## 2. Desktop Application (`Munin.exe`)

The primary executable accepts a target log file path as an optional positional argument. When provided, Munin launches and instantly maps the virtual memory pages of the file:

```cmd
Munin.exe [file_path]
```

### Usage Examples:

```powershell
# Open a local log file immediately
Munin.exe "C:\inetpub\logs\LogFiles\W3SVC1\u_ex261017.log"

# Open a massive log file from a network UNC share
Munin.exe "\\fileserver\production\logs\cluster_app.log"

# Run via Windows Terminal execution alias (via Microsoft Store / MSIX package)
munin "D:\Logs\production_backend_10gb.log"
```

---

## 3. Standalone Command-Line Interface (`munin-app-cli.exe`)

The executable `munin-app-cli.exe` is built from `source/cli/main_cli.cpp` and runs in pure headless console mode (without initializing Qt Widgets or GUI components), making it ideal for automation, CI/CD pipelines, PowerShell/Bash scripts, and remote administration:

```cmd
munin-app-cli.exe <command> [arguments...] [options...]
```

### Supported Commands:

| Command | Description | Syntax / Options |
| :--- | :--- | :--- |
| `search` | Performs high-speed multi-core search across the file | `munin-app-cli.exe search <file> <term> [--regex] [-i] [--limit <N>] [--format text\|json\|csv]` |
| `info` | Extracts file metadata (size, line count, SIMD architecture, speed) | `munin-app-cli.exe info <file> [--format text\|json]` |
| `tail` | Continuously monitors appended log records (similar to `tail -f`) | `munin-app-cli.exe tail <file> [-n <lines>] [-f] [--timeout <seconds>]` |
| `bench` | Runs memory-mapped I/O and parallel search throughput benchmark | `munin-app-cli.exe bench <file>` |
| `--version` / `-v` | Displays official release version and build info | `munin-app-cli.exe --version` |
| `--help` / `-h` | Prints full CLI syntax and usage examples | `munin-app-cli.exe --help` |

### Usage Examples:

```powershell
# Inspect metrics and line count of a log file
munin-app-cli.exe info "C:\Logs\production.log"

# Search for a term with structured JSON output (limited to 50 hits)
munin-app-cli.exe search "C:\Logs\app.log" "EXCEPTION" --limit 50 --format json

# Case-insensitive regular expression search
munin-app-cli.exe search "C:\Logs\web.log" "HTTP/1\.[01]\" [45]\d\d" --regex -i

# Live-tail an active log file displaying the last 20 lines
munin-app-cli.exe tail "C:\Logs\service.log" -n 20 -f
```

---

## 4. Engine Benchmark Utility (`MuninBench.exe`)

The `MuninBench.exe` tool is built from `source/engine/TestEngine.cpp` to validate hardware memory throughput and SIMD processing speed:

```cmd
MuninBench.exe [options]
```

### Benchmark Metrics Evaluated:
1. **Win32 Memory-Mapping Throughput:** Virtual address allocation and page-fault latency.
2. **SIMD Scanning Throughput:** Gigabytes processed per second via AVX2 (256-bit) and SSE4.2 (128-bit) instructions.
3. **Multi-Threaded Search Scaling:** Parallel partitioning efficiency across all available CPU hardware threads.

---

## 5. Version Integrity Utility (`find_VersionEditor.exe`)

Utility developed to audit and synchronize version numbers and release dates across all 28 repository files:

```powershell
# Check consistency across all 28 monitored files
.\build\build_Mocks_mingw\find_VersionEditor.exe --check .

# Launch the graphical version editor
.\build\build_Mocks_mingw\find_VersionEditor.exe
```

</details>
