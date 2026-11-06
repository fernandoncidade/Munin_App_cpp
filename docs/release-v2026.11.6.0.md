# Notas de Lançamento / Release Notes — v2026.11.6.0

<p align="center">
  <b>Selecione o idioma / Select language:</b><br>
  <a href="#ptbr">🇧🇷 Português (BR)</a> | <a href="#enus">🇺🇸 English (US)</a>
</p>

---

## <a id="ptbr"></a>🇧🇷 Português (BR)

<details>
<summary>Clique para expandir / recolher as notas de versão em português</summary><br>

# Munin App — Release v2026.11.6.0

**Munin App v2026.11.6.0**

**Tag de Versão:** `v2026.11.6.0`  
**Data de Publicação:** 6 de novembro de 2026  
**Autor:** Fernando Nillsson Cidade  
**Download Direto:** [Baixar Última Versão no GitHub](../../releases/latest) | [Instalar via Microsoft Store](https://apps.microsoft.com/detail/9PP4GVHR5THC)  

---

## 1. Artefatos de Instalação e Hashes Criptográficos

Valide a integridade do pacote baixado antes da execução utilizando o cmdlet nativo do PowerShell:

```powershell
Get-FileHash -Path ".\Munin-v2026.11.6.0-x64-Setup.exe" -Algorithm SHA256
```

| Nome do Artefato | Formato | Arquitetura Alvo | Descrição |
|---|---|:---:|---|
| `Munin-v2026.11.6.0-x64-Setup.exe` | Instalador Inno Setup | x64 (MSVC / MinGW) | Instalador gráfico completo com integração ao Windows Explorer e atalhos na Área de Trabalho. |
| `Munin-v2026.11.6.0-x64-Portable.zip` | Arquivo ZIP Portátil | x64 | Versão portátil sem necessidade de instalação administrativa. |
| `Source code (zip)` | Arquivo ZIP | Qualquer | Código-fonte completo em C++20 e Qt 6. |
| `Source code (tar.gz)` | Arquivo Tarball | Qualquer | Código-fonte completo em C++20 e Qt 6. |

---

## 2. Destaques da Versão

- **Abertura Ultrarrápida Zero-Copy:** Mapeamento de memória virtual Win32 (`CreateFileMappingW` e `MapViewOfFile`) abrindo arquivos de múltiplos gigabytes em frações de segundo.
- **Indexação Vetorial SIMD:** Escaneamento de delimitadores de linha (`\n`) acelerado por instruções AVX2 e SSE4.2 atingindo velocidades superiores a 3 GB/s.
- **Monitoramento em Tempo Real (Live Tail):** Acompanhamento contínuo de registros via notificações de kernel `ReadDirectoryChangesW` sem ciclos de polling.
- **Busca Paralela Multi-Core:** Divisão de trabalho entre todos os núcleos de hardware da CPU, com suporte a Expressões Regulares JIT e filtragem dinâmica de linhas.
- **Histograma Temporal (Time-Slice):** Detecção automática de timestamps ISO-8601 e visualização gráfica de picos de incidentes e anomalias.
- **Inspetor Estruturado de JSON:** Painel lateral retrátil com formatação automática, realce sintático e árvore interativa de nós para logs estruturados.
- **Minimapa de Densidade:** Barra de rolagem lateral com mapa de calor cromático indicando erros, avisos e resultados de pesquisa ao longo de todo o documento.
- **Temas Nativos e Efeito Mica:** Integração completa com o tema Claro e Escuro do Windows 11 via DWM e personalização nativa Qt6.
- **Governança Aberta e Duplo Licenciamento:** Código-fonte aberto 100% livre e transparente regido pela GNU AGPLv3, com esclarecimento e salvaguardas expressas no modelo de duplo licenciamento para a Microsoft Store.

---

## 3. Requisitos de Sistema

- **Sistema Operacional:** Windows 11 / Windows 10 (64-bit x64).
- **Processador:** CPU x86-64 com suporte a instruções SSE4.2 (instruções AVX2 altamente recomendadas para máxima performance).
- **Memória RAM:** Mínimo de 128 MB (consumo típico < 60 MB mesmo para arquivos gigantescos).
- **Armazenamento:** ~45 MB de espaço livre em disco.

</details>

---

## <a id="enus"></a>🇺🇸 English (US)

<details>
<summary>Click to expand / collapse release notes in English</summary><br>

# Munin App — Release v2026.11.6.0

**Munin App v2026.11.6.0**

**Release Tag:** `v2026.11.6.0`  
**Publication Date:** November 6, 2026  
**Author:** Fernando Nillsson Cidade  
**Direct Download:** [Download Latest Release on GitHub](../../releases/latest) | [Install via Microsoft Store](https://apps.microsoft.com/detail/9PP4GVHR5THC)  

---

## 1. Installation Artifacts & Cryptographic Hashes

Verify the integrity of downloaded binaries via PowerShell before execution:

```powershell
Get-FileHash -Path ".\Munin-v2026.11.6.0-x64-Setup.exe" -Algorithm SHA256
```

| Artifact Name | Format | Target Architecture | Description |
|---|---|:---:|---|
| `Munin-v2026.11.6.0-x64-Setup.exe` | Inno Setup Installer | x64 (MSVC / MinGW) | Full graphical installer with Windows Explorer integration and desktop shortcuts. |
| `Munin-v2026.11.6.0-x64-Portable.zip` | Portable ZIP Archive | x64 | Portable zero-install edition. Unzip and run immediately. |
| `Source code (zip)` | ZIP Archive | Any | Full C++20 and Qt 6 source code. |
| `Source code (tar.gz)` | Tarball Archive | Any | Full C++20 and Qt 6 source code. |

---

## 2. Release Highlights

- **Ultra-Fast Zero-Copy Memory Mapping:** Native Win32 `CreateFileMappingW` and `MapViewOfFile` opening multi-gigabyte files instantaneously.
- **SIMD Vectorized Indexing:** Newline delimiter (`\n`) scanning accelerated with AVX2 and SSE4.2 instructions exceeding 3 GB/s throughput.
- **Real-Time Live Tail Monitoring:** Asynchronous kernel notification tracking (`ReadDirectoryChangesW`) with zero idle polling loops.
- **Multi-Core Parallel Search:** Log chunk partitioning across all logical CPU threads with JIT regular expression acceleration and dynamic row filtering.
- **Time-Slice Temporal Histogram:** Automatic ISO-8601 timestamp parsing with an interactive incident density histogram.
- **Structured JSON Inspector:** Collapsible side panel rendering structured logs with syntax highlighting and interactive tree nodes.
- **Minimap Heatmap Scrollbar:** Visual density scrollbar with color-coded markers for errors, warnings, and search hits.
- **Native Themes & Mica Integration:** Full dynamic Windows 11 Light and Dark mode synchronization via Win32 DWM.
- **Open Governance & Dual-Licensing:** Complete open-source codebase governed under GNU AGPLv3 with clear dual-licensing safeguards for Microsoft Store deployment and robust compliance transparency.

---

## 3. System Requirements

- **Operating System:** Windows 11 / Windows 10 (64-bit x64).
- **Processor:** x86-64 CPU supporting SSE4.2 (AVX2 strongly recommended for optimal vector throughput).
- **RAM:** Minimum 128 MB (typical runtime consumption < 60 MB even on massive files).
- **Disk Space:** ~45 MB free storage.

</details>
