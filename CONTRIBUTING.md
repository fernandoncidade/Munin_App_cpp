# Guia de Contribuição / Contributing Guide — Munin App

<p align="center">
  <b>Selecione o idioma / Select language:</b><br>
  <a href="#ptbr">🇧🇷 Português (BR)</a> | <a href="#enus">🇺🇸 English (US)</a>
</p>

---

## <a id="ptbr"></a>🇧🇷 Português (BR)

<details>
<summary>Clique para expandir / recolher as instruções em português</summary><br>

# Contribuindo com o Munin App

Agradecemos imensamente seu interesse em contribuir com o **Munin App — Leitor de Logs Massivos**! Seu apoio fortalece o processamento de dados em altíssimo desempenho, auditoria e estabilidade da visualização de arquivos gigantescos no Windows.

> [!NOTE]
> **Sobre o Desenvolvimento, Suporte e Contribuições Externas:**  
> Este projeto é concebido, desenvolvido e mantido primariamente como uma iniciativa pessoal e educacional pelo autor, que é iniciante e está em processo contínuo de aprendizado em programação.  
> O envio de sugestões, abertura de *Issues* e submissão de *Pull Requests (PRs)* são sempre bem-vindos! No entanto, ressalta-se que a análise de código, interação em tópicos técnicos e incorporação de contribuições são realizadas no próprio ritmo do autor e de forma estritamente voluntária. Caso um chamado ou PR demore para ser respondido ou não venha a ser incorporado à branch principal, por favor não reaja com alarde ou frustração. Conforme estabelecido pela licença GNU AGPLv3, você possui total liberdade para criar um *Fork* independente deste repositório e evoluir suas próprias ideias e versões.

---

## 1. Código de Conduta

Ao participar deste projeto, você concorda em cumprir nosso [Código de Conduta](CODE_OF_CONDUCT.md). Trate todos os colaboradores com respeito, cortesia e postura profissional.

---

## 2. Formas de Contribuir

Você pode colaborar com o Munin App de diversas maneiras:
- **Relatar Bugs:** Submeta relatórios claros com amostras de arquivos de log, arquitetura de CPU (AVX2/SSE4.2) e comportamento observado.
- **Sugerir Funcionalidades:** Proponha novos recursos de análise temporal, heurísticas para formatos de log específicos (Syslog, Nginx, IIS, JSON estruturado) ou melhorias na interface gráfica.
- **Otimizações de Baixo Nível:** Aprimore algoritmos SIMD, gerência de páginas de memória virtual e I/O de disco assíncrono.
- **Melhorar a Documentação:** Esclareça orientações de compilação, manuais de usuário e modelos de segurança.
- **Traduções (i18n):** Auxilie na adição de novos idiomas aos arquivos `.ts` e `.qm` do Qt.

---

## 3. Configuração do Ambiente de Desenvolvimento

### 3.1. Pré-requisitos
- **Sistema Operacional:** Windows 10 / 11 x64 ou Windows Server 2022+
- **Compilador:**
  - Microsoft Visual C++ (MSVC) do Visual Studio 2022 (toolset v143) com suporte a C++20
  - Ou MinGW GCC 13+ (x86_64-w64-mingw32)
- **CMake:** Versão 3.21 ou superior
- **Qt:** Qt 6.8+ ou Qt 6.11+ (módulos Core, Gui, Widgets)
- **Ferramenta de Build:** Ninja (recomendado)

### 3.2. Compilando com Presets do CMake

Disponibilizamos presets oficiais no arquivo `CMakePresets.json`:

```powershell
# Clonar o repositório
git clone https://github.com/fernandoncidade/Munin_App_cpp.git
cd Munin_App_cpp

# Configurar e compilar a aplicação com MinGW Ninja
cmake --preset host-qt6-mingw-release
cmake --build --preset host-qt6-mingw-release --parallel

# Ou compilar com MSVC (Visual Studio 2022)
cmake --preset host-qt6-msvc-release
cmake --build --preset host-qt6-msvc-release --parallel

# Compilar o utilitário de benchmark MuninBench
cmake --preset host-qt6-bench-mingw-release
cmake --build --preset host-qt6-bench-mingw-release --parallel

# Compilar os utilitários e mocks (incluindo find_VersionEditor)
cmake --preset mocks-mingw-release
cmake --build --preset mocks-mingw-release --parallel
```

---

## 4. Padrões de Código e Diretrizes Arquiteturais

1. **C++20 Moderno e Seguro:**
   - Adote RAII estrito para recursos do sistema operacional (`HANDLE`, visualizações mapeadas de memória virtual).
   - Mantenha a biblioteca estática `MuninEngine` 100% desacoplada de dependências de interface gráfica (sem includes de Qt).
2. **Vetorização e SIMD:**
   - Sempre forneça rotinas com detecção em tempo de execução e fallback robusto (`AVX2 -> SSE4.2 -> Escalar`).
   - Garanta alinhamentos de memória adequados ao utilizar intrínsecos de 256 bits.
3. **Privacidade e Operação Offline (Local-First):**
   - Não introduza dependências de rede, serviços em nuvem ou bibliotecas de telemetria externa. O Munin opera estritamente local e offline.
4. **Gerenciamento de Janelas e UI:**
   - Diálogos auxiliares devem seguir o padrão Modeless desacoplado implementado em `GerenciadorDialogos`.
   - Assegure compatibilidade visual com os temas Claro e Escuro e integração DWM.

---

## 5. Fluxo de Trabalho com Git e Pull Requests

1. Crie uma branch a partir da `main`:
   ```bash
   git checkout -b feature/minha-melhoria-ou-correcao
   ```
2. Realize commits atômicos com mensagens objetivas e claras.
3. Execute o benchmark `MuninBench` e o verificador `find_VersionEditor --check .` antes de abrir a PR:
   ```powershell
   ./build/build_Mocks_mingw/find_VersionEditor.exe --check .
   ```
4. Submeta o Pull Request utilizando nosso template oficial [.github/PULL_REQUEST_TEMPLATE.md](.github/PULL_REQUEST_TEMPLATE.md).

</details>

---

## <a id="enus"></a>🇺🇸 English (US)

<details>
<summary>Click to expand / collapse instructions in English</summary><br>

# Contributing to Munin App

Thank you for your interest in contributing to **Munin App — Massive Log Viewer**! Your contributions help maintain a rock-solid, lightning-fast log viewer and analyzer on Windows.

> [!NOTE]
> **About Project Development, Support, and External Contributions:**  
> This project is designed, developed, and maintained primarily as a personal and educational initiative by the author, who is a beginner and in an ongoing learning journey in software engineering.  
> Feedback, suggestions, *Issues*, and *Pull Requests (PRs)* are always welcomed from the community! However, please be aware that reviewing technical inquiries, code submissions, and upstream merges is carried out strictly at the author's own pace on a voluntary basis. If a ticket or PR is not answered promptly or is not merged into the mainline branch, please do not react with alarm or frustration. Under the terms of the GNU AGPLv3 License, you have complete freedom to create an independent *Fork* of this repository to evolve your own ideas and customized builds.

---

## 1. Code of Conduct

By participating, you agree to adhere to our [Code of Conduct](CODE_OF_CONDUCT.md). Please treat all contributors with respect, courtesy, and professionalism.

---

## 2. Ways to Contribute

You can collaborate with Munin App in several ways:
- **Report Bugs:** Submit descriptive reports including sample log line structures, CPU architecture (AVX2/SSE4.2), and observed behavior.
- **Suggest Features:** Propose ideas for time-slice analysis, log format parsers (Syslog, Nginx, IIS, JSON), or UI improvements.
- **Low-Level Optimizations:** Enhance SIMD line-scanning algorithms, virtual memory paging, and asynchronous I/O.
- **Documentation:** Improve installation instructions, user manuals, and technical specifications.
- **Localization (i18n):** Help translate Qt translation files (`.ts` and `.qm`) into additional languages.

---

## 3. Development Environment Setup

### 3.1. Prerequisites
- **Operating System:** Windows 10 / 11 x64 or Windows Server 2022+
- **Compiler:**
  - Microsoft Visual C++ (MSVC) from Visual Studio 2022 (toolset v143) with C++20 support
  - Or MinGW GCC 13+ (x86_64-w64-mingw32)
- **CMake:** Version 3.21 or later
- **Qt:** Qt 6.8+ or Qt 6.11+ (Core, Gui, Widgets modules)
- **Build Tool:** Ninja (recommended)

### 3.2. Building with CMake Presets

Official presets are provided in `CMakePresets.json`:

```powershell
# Clone the repository
git clone https://github.com/fernandoncidade/Munin_App_cpp.git
cd Munin_App_cpp

# Configure and build the application with MinGW Ninja
cmake --preset host-qt6-mingw-release
cmake --build --preset host-qt6-mingw-release --parallel

# Or build with MSVC (Visual Studio 2022)
cmake --preset host-qt6-msvc-release
cmake --build --preset host-qt6-msvc-release --parallel

# Build the MuninBench benchmark utility
cmake --preset host-qt6-bench-mingw-release
cmake --build --preset host-qt6-bench-mingw-release --parallel

# Build mocks and utilities (including find_VersionEditor)
cmake --preset mocks-mingw-release
cmake --build --preset mocks-mingw-release --parallel
```

---

## 4. Coding Standards & Architecture Guidelines

1. **Modern and Safe C++20:**
   - Adhere strictly to RAII for system resources (`HANDLE`, mapped views of virtual memory).
   - Keep the `MuninEngine` static library completely independent of Qt and graphical libraries.
2. **Vectorization and SIMD:**
   - Always provide runtime CPU feature detection with robust fallback (`AVX2 -> SSE4.2 -> Scalar`).
   - Ensure proper memory alignment when using 256-bit SIMD intrinsics.
3. **Privacy and Offline Operation (Local-First):**
   - Do not introduce network calls, cloud dependencies, or external telemetry. Munin runs entirely locally and offline.
4. **Window Management & UI:**
   - Auxiliary dialogs must follow the modeless pattern orchestrated by `GerenciadorDialogos`.
   - Ensure complete visual compatibility with Windows 11 Light and Dark modes, including DWM integration.

---

## 5. Git Workflow & Pull Requests

1. Create a branch from `main`:
   ```bash
   git checkout -b feature/my-improvement-or-fix
   ```
2. Make atomic commits with clear, descriptive commit messages.
3. Run the `MuninBench` tool and verify all monitored files with `find_VersionEditor --check .`:
   ```powershell
   ./build/build_Mocks_mingw/find_VersionEditor.exe --check .
   ```
4. Open a Pull Request filling out our [.github/PULL_REQUEST_TEMPLATE.md](.github/PULL_REQUEST_TEMPLATE.md).

</details>
