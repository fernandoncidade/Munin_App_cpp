# Munin App — Leitor de Logs Massivos para Windows

[![Versão Oficial](https://img.shields.io/badge/Versão-2026.11.6.0-blue.svg)](RELEASE.txt) [![Official Version](https://img.shields.io/badge/Version-2026.11.6.0-blue.svg)](RELEASE.txt) [![Microsoft Store](https://img.shields.io/badge/Microsoft_Store-Dispon%C3%ADvel-0078D4?logo=microsoft)](https://apps.microsoft.com/detail/9PP4GVHR5THC) [![C++20](https://img.shields.io/badge/C%2B%2B-20-00599C?logo=c%2B%2B)](CMakeLists.txt) [![Qt 6.11.1](https://img.shields.io/badge/Qt-6.11.1-41CD52?logo=qt)](CMakeLists.txt) [![Windows 11 Native](https://img.shields.io/badge/Plataforma-Windows_11-0078D4?logo=windows)](https://microsoft.com) [![Licença GNU AGPLv3](https://img.shields.io/badge/Licen%C3%A7a-AGPLv3-blue.svg)](LICENSE) [![Segurança 100% Offline](https://img.shields.io/badge/Seguran%C3%A7a-100%25_Offline-success.svg)](MANUAL.md)


---

<!-- Multilanguage README.md for Munin App -->

<p align="center">
  <b>Selecione o idioma / Select language:</b><br>
  <a href="#ptbr">Português (BR)</a> |
  <a href="#enus">English (US)</a>
</p>

---

## <a id="ptbr"></a>Português (BR)

> **Observação:** Este repositório refere-se à versão oficial **v2026.11.6.0** do projeto **Munin App**. Apoie o projeto e adquira a versão para Windows por meio do link oficial: [Instalar via Microsoft Store](https://apps.microsoft.com/detail/9PP4GVHR5THC)

<details>
<summary>Clique para expandir / recolher</summary><br>

# Munin App — Leitor de Logs Massivos

Versão: v2026.11.6.0<br>
Data técnica desta revisão: 6 de novembro de 2026<br>
Autor: Fernando Nillsson Cidade<br>

## Resumo

O **Munin App** é um visualizador e analisador de arquivos de log massivos de altíssimo desempenho para Windows 11, projetado e construído nativamente em **C++20** com interface gráfica moderna em **Qt 6**. 

Projetado especificamente para eliminar o clássico gargalo onde editores e visualizadores convencionais congelam, demoram minutos para indexar ou esgotam a memória RAM ao tentar abrir arquivos de centenas de megabytes a dezenas de gigabytes, o Munin App utiliza uma arquitetura estritamente virtualizada de ponta a ponta:
- **Arquivos Mapeados em Memória Win32 (Memory-Mapped Files)**: leitura *zero-copy* sob demanda controlada diretamente pelo subsistema de paginação do kernel do Windows NT (`CreateFileMappingW` e `MapViewOfFile`), permitindo abrir arquivos de 1 GB, 10 GB ou centenas de gigabytes em frações de segundo com consumo mínimo de memória física;
- **Indexação Vetorial por Instruções SIMD (AVX2 e SSE4.2)**: algoritmo otimizado de escaneamento de delimitadores de linha (`\n`) atingindo taxas de transferência superiores a 3 GB/s;
- **Monitoramento em Tempo Real (Live Tail)**: acompanhamento contínuo de registros anexados concorrentemente usando notificações assíncronas do kernel (`ReadDirectoryChangesW`) sem loops de polling;
- **Busca Paralela Multi-Core**: partição do arquivo em blocos distribuídos por todas as threads lógicas da CPU, com suporte a Expressões Regulares (Regex JIT), diferenciação de maiúsculas/minúsculas e filtragem dinâmica de linhas correspondentes;
- **Histograma Temporal de Volume (Time-Slice Scanner)**: identificação automática de carimbos de data/hora (ISO 8601) com gráfico de densidade para detecção instantânea de picos de eventos e surtos de erro;
- **Inspetor Estruturado de Registros e JSON**: análise sintática e exibição em árvore hierárquica para logs estruturados modernos;
- **Interface e Tema Adaptativo**: integração nativa com o Windows 11 (Mica, barras de título escuras/claras DWM) e internacionalização dinâmica em tempo de execução entre Português (Brasil) e Inglês (Estados Unidos).

---

## Funcionalidades Principais

| Recurso | Descrição Técnica |
| :--- | :--- |
| **Abertura Instantânea de Logs** | Abertura imediata de arquivos gigantescos sem carregamento prévio na RAM heap, utilizando mapeamento virtual de páginas em memória Win32. |
| **Indexação SIMD de Linhas** | Processamento vetorial de 256 bits (AVX2) e 128 bits (SSE4.2) com fallback escalar para indexar offsets de linhas a mais de 3 GB/s. |
| **Indexador por Chunks (ChunkIndex)** | Divisão esparsa em blocos de 64 KB com tabela de ponteiros, permitindo acesso aleatório $O(1)$ a qualquer linha do documento. |
| **Navegação Virtualizada (LogView)** | Tabela customizada baseada em `QAbstractItemView` e `QTableView` do Qt6, que instancia graficamente apenas as linhas visíveis na viewport. |
| **Minimapa de Densidade (MinimapScrollBar)** | Barra de rolagem interativa com renderização de densidade de linhas e marcadores visuais de resultados de pesquisa e níveis de erro. |
| **Monitoramento Live Tail** | Modo de acompanhamento em tempo real (`Ctrl+L`) com rolagem automática sincronizada via eventos do subsistema de arquivos do Windows. |
| **Busca Paralela Multithread** | Mecanismo de busca (`Ctrl+F`) que particiona o arquivo entre todos os núcleos lógicos, com suporte a literais, regex e modo de filtragem. |
| **Análise Temporal (Time-Slice)** | Histograma de distribuição temporal (`Ctrl+T`) para isolamento de períodos críticos de atividade e incidentes operacionais. |
| **Inspetor de JSON e Detalhes** | Painel retrátil (`Ctrl+J`) para inspeção de linhas complexas, formatando payloads JSON estruturados em árvore navegável com realce sintático. |
| **Gerador Sintético de Logs** | Ferramenta embutida para criar arquivos sintéticos padronizados de 100 MB, 500 MB e 1 GB para testes de carga e benchmarks. |
| **Utilitário de Terminal (CLI)** | Ferramenta standalone de linha de comando (`munin-app-cli.exe`) para inspeção de métricas, busca paralela e streaming de logs via terminal. |
| **Tema Automático Windows 11** | Detecção e alternância dinâmica em tempo de execução dos modos Claro e Escuro, com barra de títulos nativa estilizada via Windows DWM. |
| **Internacionalização em Tempo Real** | Troca instantânea entre Português (Brasil) e English (United States) sem reiniciar a aplicação. |
| **Privacidade e Soberania de Dados** | Operação 100% autônoma e offline. Nenhum dado, caminho, termo de busca ou arquivo é enviado para servidores externos. |

---

## 📸 Capturas de Tela da Interface (UI)

<div align="center">
  <table>
    <tr>
      <td align="center">
        <b>Visualizador de Logs & Minimapa de Densidade</b><br>
        <img src="repo/LISTAGEM/pt_BR/Captura%20de%20tela%202026-10-17%20023534.png" width="400" alt="Visualizador de Logs & Minimapa" /><br>
        <i>Navegação virtualizada fluida com renderização instantânea</i>
      </td>
      <td align="center">
        <b>Busca Paralela Multi-Core & Filtragem JIT</b><br>
        <img src="repo/LISTAGEM/pt_BR/Captura%20de%20tela%202026-10-17%20023653.png" width="400" alt="Busca Paralela Multi-Core" /><br>
        <i>Localização rápida de padrões e destaque cromático no minimapa</i>
      </td>
    </tr>
    <tr>
      <td align="center" colspan="2">
        <b>Histograma Temporal (Time-Slice) & Inspetor JSON Estruturado</b><br>
        <img src="repo/LISTAGEM/pt_BR/Captura%20de%20tela%202026-10-17%20023713.png" width="600" alt="Histograma Temporal e Inspetor JSON" /><br>
        <i>Detecção de surtos temporais e dissecção hierárquica de payloads</i>
      </td>
    </tr>
  </table>
</div>

---

## 📚 Documentação Técnica Adicional

- 📐 **[Arquitetura do Sistema](docs/architecture.md):** Engenharia Win32 zero-copy, SIMD AVX2/SSE4.2 e modelos virtuais Qt 6.
- 💻 **[Referência da Linha de Comando](docs/cli-reference.md):** Execução via terminal, parâmetros de inicialização e utilitário de benchmark `MuninBench`.
- 🛡️ **[Modelo de Segurança e Ameaças](docs/security-model.md):** Garantias de integridade somente leitura, prevenção de buffer overflow e operação 100% offline.
- ⚖️ **[Mapeamento de Conformidade](docs/compliance.md):** Conformidade com NIST SP 800-92, PCI-DSS v4.0, ISO 27001, LGPD e GDPR.
- 📜 **[Licenciamento e Modelo Comercial](docs/licensing.md):** Licença GNU AGPLv3 do código-fonte, distribuição na Microsoft Store e liberdade total para desenvolvedores.
- 📖 **[Guia do Usuário e Operação](docs/user-guide.md):** Manual operacional passo a passo para visualização, Live Tail e análise estruturada.
- 📦 **[Notas de Lançamento](docs/release-v2026.11.6.0.md):** Histórico da versão, hashes SHA-256 e artefatos de distribuição.
- 💡 **[Sugestões de Issues Iniciais](docs/suggested-issues.md):** Propostas estruturadas para colaboração e desenvolvimento da comunidade.
- 🤝 **[Guia de Contribuição](CONTRIBUTING.md)** | 📜 **[Código de Conduta](CODE_OF_CONDUCT.md)** | 🔒 **[Política de Segurança](SECURITY.md)**

---

## Destaques Técnicos da Versão 2026.10.19.0

1. **Arquitetura da Engine C++20 Desacoplada (`MuninEngine`)**:
   - Isolamento completo da lógica de I/O de baixo nível, indexação vetorial e busca em uma biblioteca estática independente de bibliotecas gráficas.
   - Suporte a ponteiros diretos de memória mapeada com tratamento seguro de exceções de acesso a arquivos.

2. **Indexação Vetorial de Altíssimo Desempenho (`SimdUtils.cpp`)**:
   - Detecção automática de suporte a instruções AVX2 em tempo de execução.
   - Utilização de registradores `__m256i` e máscaras de bits (`_mm256_movemask_epi8`) para localizar quebras de linha (`\n`) 32 bytes por ciclo de CPU.
   - Fallback de alta fidelidade para SSE4.2 (`_mm_movemask_epi8` com 16 bytes por ciclo) e rotina escalar otimizada.

3. **Busca Paralela Multi-Core (`ParallelSearch.cpp`)**:
   - Criação de pool de threads adaptado ao número de núcleos lógicos disponíveis (`std::thread::hardware_concurrency()`).
   - Cada thread processa um intervalo de blocos pré-alocados de 64 KB, reduzindo a contenção de memória e evitando bloqueios de lock contention.

4. **Monitoramento Live Tail via Win32 Nativo (`LiveTailWatcher.cpp`)**:
   - Acompanhamento passivo de crescimento de arquivos por meio da API Win32 `ReadDirectoryChangesW` em thread dedicada.
   - Atualização contínua de offsets de novas linhas sem reinicializar o índice do arquivo já mapeado.

5. **Gerenciamento de Diálogos Modeless e Z-Order Independente (`GerenciadorDialogos.cpp`)**:
   - Implementação de janelas de diálogo independentes (`SobreDialog` e `ManualDialog`) desacopladas perante o gerenciador de janelas do sistema operacional.
   - Permite alternar livremente o foco e a sobreposição (Z-Order) entre a interface principal e os diálogos abertos, garantindo máxima usabilidade em estações de trabalho multi-monitor.

6. **Sincronização Completa com o Utilitário Version Editor (`mocks/find_VersionEditor.cpp`)**:
   - Validação e atualização automatizada e consistente de versão e data em todos os arquivos de código, políticas de privacidade, termos de uso e documentação do projeto.

7. **Utilitário de Linha de Comando Autônomo (`munin-app-cli`)**:
   - Executável independente para processar arquivos de log diretamente via PowerShell, CMD ou Windows Terminal sem carregar a pilha gráfica do Qt;
   - Suporte completo a comandos de inspeção (`info`), busca multi-core acelerada por SIMD (`search`), streaming ao vivo sem polling (`tail`) e benchmarks integrados (`bench`);
   - Vinculação com `AppExecutionAlias` no pacote MSIX e instaladores Inno Setup para invocação global em qualquer diretório do sistema.

---

## Atalhos de Teclado Principais

| Atalho | Ação Correspondente |
| :--- | :--- |
| `Ctrl+O` | Abrir arquivo de log no visualizador |
| `Ctrl+F` | Alternar exibição da barra de busca rápida |
| `Ctrl+T` | Alternar exibição da linha do tempo / Time-Slice |
| `Ctrl+J` | Alternar exibição do inspetor de JSON estruturado |
| `Ctrl+L` | Ativar ou desativar o modo Live Tail (acompanhamento ao vivo) |
| `Ctrl+G` | Abrir diálogo de geração de logs sintéticos de teste |
| `Esc` | Limpar filtros ativos de busca ou fechar painéis retráteis |
| `F1` | Abrir o Manual de Instruções completo da aplicação |
| `Alt+A` | Abrir o menu "Arquivo" |
| `Alt+E` | Abrir o menu "Exibir" |
| `Alt+C` | Abrir o menu "Configurações" |
| `Alt+B` ou `Alt+S` | Abrir o menu "Sobre" |

---

## Requisitos do Sistema

- **Sistema Operacional**: Windows 10 (64-bit) versão 21H2 ou Windows 11 (64-bit, recomendado).
- **Processador**: Intel Core i3/i5/i7/i9 ou AMD Ryzen x64 com suporte a instruções SSE4.2 (AVX2 altamente recomendado para máximo rendimento SIMD).
- **Memória RAM**: 4 GB no mínimo (a arquitetura de arquivos mapeados em memória requer apenas frações de megabytes de RAM mesmo para arquivos de dezenas de gigabytes).
- **Armazenamento**: 150 MB livres em disco para instalação da distribuição binária completa.

---

## Compilação e Build

### Ferramentas Necessárias
- **CMake**: versão 3.21 ou superior.
- **Ninja**: gerador de build de alta velocidade.
- **Qt 6**: Qt 6.11.1 (módulos Core, Gui, Widgets).
- **Compiladores homologados**:
  - MinGW 64-bit (GCC 13.1.0+);
  - Microsoft Visual C++ (MSVC 2022 v143+).

### 1. Build com MinGW (Preset `host-qt6-mingw-release`)

```powershell
# Configurar o projeto com preset MinGW
cmake --preset host-qt6-mingw-release

# Compilar e executar o deploy completo do runtime
cmake --build --preset host-qt6-mingw-release --target MuninDeploy --parallel
```

O executável e dependências serão disponibilizados em:
- `build/build_Munin_mingw/Munin.exe`
- Distribuição limpa externa: `D:\MISCELANEAS\VSCode\Munin\Munin_mingw\`

### 2. Build com MSVC (Preset `host-qt6-msvc-release`)

```powershell
# Configurar o projeto com preset MSVC via script de ambiente
cmd.exe /c "cmake\run_msvc.bat cmake --preset host-qt6-msvc-release"

# Compilar e executar o deploy completo do runtime
cmd.exe /c "cmake\run_msvc.bat cmake --build --preset host-qt6-msvc-release --target MuninDeploy --parallel"
```

O executável e dependências serão disponibilizados em:
- `build/build_Munin_msvc/Munin.exe`
- Distribuição limpa externa: `D:\MISCELANEAS\VSCode\Munin\Munin_msvc\`

### 3. Utilitários e Mocks (`mocks/find_VersionEditor`)

Para compilar e verificar o utilitário gerenciador de versões:

```powershell
# Compilação dos Mocks com MSVC
cmd.exe /c "cmake\run_msvc.bat cmake --build --preset host-qt6-mocks-msvc-release --parallel"

# Verificação de integridade dos arquivos (CLI)
build\build_Mocks_msvc\find_VersionEditor.exe --check
```

---

## Estrutura do Repositório

```text
Munin_App_cpp/
├── .github/                       # Workflows de CI/CD do GitHub Actions e templates
│   ├── workflows/build.yml        # Pipeline de compilação automatizada (MinGW e MSVC)
│   ├── ISSUE_TEMPLATE/            # Templates para reporte de bugs e solicitações de recursos
│   └── PULL_REQUEST_TEMPLATE.md   # Modelo padronizado para submissão de pull requests
├── docs/                          # Documentação técnica modular e especificações de conformidade
│   ├── architecture.md            # Arquitetura Win32 zero-copy, SIMD AVX2/SSE4.2 e Qt6
│   ├── cli-reference.md           # Referência completa da CLI munin-app-cli e MuninBench
│   ├── compliance.md              # Mapeamento de conformidade NIST SP 800-92, LGPD e GDPR
│   ├── licensing.md               # Modelo de licenciamento GNU AGPLv3 e autorização Microsoft Store
│   ├── release-v2026.11.6.0.md    # Notas técnicas de lançamento, hashes SHA-256 e artefatos
│   ├── security-model.md          # Modelo de segurança, análise de ameaças e integridade
│   ├── suggested-issues.md        # Backlog estruturado de issues para a comunidade
│   └── user-guide.md              # Guia operacional passo a passo do usuário
├── repo/                          # Scripts de instalação Inno Setup, manifesto MSIX e assets
│   └── LISTAGEM/                       # Capturas de tela oficiais da Microsoft Store (pt_BR/en_US)
├── mocks/                         # Utilitários de desenvolvimento e ferramentas auxiliares
│   ├── CMakeLists.txt                  # Configuração de build dos alvos auxiliares
│   ├── CompileTranslationsGUI.hpp/.cpp # Interface gráfica de compilação de catálogos .ts -> .qm
│   ├── CompileTranslationsTerminal.hpp/.cpp # Compilador de traduções em linha de comando
│   ├── find_VersionEditor.hpp/.cpp     # Utilitário de sincronização e validação de versões
│   └── icons/                          # Ícones e recursos visuais dos utilitários
├── source/                        # Código-fonte principal da aplicação
│   ├── main.cpp                   # Ponto de entrada do executável Munin (GUI)
│   ├── cli/                       # Utilitário Standalone de Linha de Comando (CLI Headless)
│   │   └── main_cli.cpp           # Ponto de entrada de munin-app-cli.exe
│   ├── engine/                    # Motor de processamento de baixo nível Win32/SIMD
│   │   ├── MemoryMappedFile.hpp/.cpp   # Mapeamento de arquivos em memória Win32 (zero-copy)
│   │   ├── SimdUtils.hpp/.cpp          # Vetorização AVX2/SSE4.2 para escaneamento de linhas
│   │   ├── ChunkIndex.hpp/.cpp         # Índice esparso por blocos virtuais de 64 KB
│   │   ├── LiveTailWatcher.hpp/.cpp    # Monitoramento de escrita concorrente em tempo real
│   │   ├── ParallelSearch.hpp/.cpp     # Busca paralela multithread multi-core
│   │   ├── TimeSliceScanner.hpp/.cpp   # Parser de carimbos temporais e histograma
│   │   └── TestEngine.cpp              # Target de benchmark MuninBench
│   ├── model/                     # Modelo de dados virtualizado
│   │   └── LogModel.hpp/.cpp           # QAbstractTableModel sob demanda para o Qt6
│   ├── ui/                        # Interface gráfica Qt6 moderna e responsiva
│   │   ├── MainWindow.hpp/.cpp         # Janela principal, orquestração e barra de status
│   │   ├── LogView.hpp/.cpp            # Visualizador customizado de tabela virtualizada
│   │   ├── LogItemDelegate.hpp/.cpp    # Delegado customizado de renderização e realce de células
│   │   ├── MinimapScrollBar.hpp/.cpp   # Barra de rolagem com minimapa de densidade
│   │   ├── SearchBarWidget.hpp/.cpp    # Painel de busca com regex e contadores de ocorrências
│   │   ├── TimeSliceWidget.hpp/.cpp    # Widget de histograma temporal interativo
│   │   ├── JsonInspectorWidget.hpp/.cpp# Inspetor de JSON estruturado em árvore
│   │   ├── LogGeneratorDialog.hpp/.cpp # Gerador de arquivos de log sintéticos para testes
│   │   ├── StickyMenu.hpp/.cpp         # Menus modernos com comportamento fluido
│   │   ├── ThemeManager.hpp/.cpp       # Gerenciador de temas Claro / Escuro Windows 11
│   │   ├── Win32DwmHelper.hpp/.cpp     # Integração com Windows DWM (Mica e Dark Title Bar)
│   │   ├── GerenciadorDialogos.hpp/.cpp# Gerenciador de diálogos desacoplados (Z-Order)
│   │   ├── SobreDialog.hpp/.cpp        # Caixa de diálogo Sobre com histórico e licenças
│   │   ├── OpcoesSobre.hpp/.cpp        # Seletor de opções avançadas do diálogo Sobre
│   │   ├── ButtonUtils.hpp/.cpp        # Utilitários e estilos de botões da interface
│   │   └── Manual.hpp/.cpp             # Diálogo interativo do Manual de Instruções
│   ├── language/                  # Catálogos e gerenciador de tradução dinâmica
│   │   ├── GerenciadorTraducao.hpp/.cpp# Carregamento e troca dinâmica de arquivos .qm
│   │   └── translations/               # Arquivos de tradução (munin_pt_BR.ts/.qm, munin_en_US.ts/.qm)
│   ├── assets/                    # Textos informativos, termos de licença, políticas e ícones
│   │   ├── ABOUT/                      # Textos do Sobre e histórico institucional
│   │   ├── CLC/                        # Contratos de licença comercial corporativa (CLC)
│   │   ├── COPYRIGHT/                  # Avisos formais de copyright e marca registrada
│   │   ├── EULA/                       # Licenças de uso de usuário final em PT-BR e EN-US
│   │   ├── LICENSES/                   # Textos integrais das licenças upstream e componentes
│   │   ├── NOTICES/                    # Avisos de software de terceiros e atribuições
│   │   ├── PRIVACY_POLICY/             # Políticas de privacidade em conformidade com LGPD/GDPR
│   │   ├── RELEASE/                    # Notas técnicas de lançamento oficial da versão
│   │   └── icons/                      # Ícones em formato .ico e .png em múltiplas resoluções
│   └── utils/                     # Utilitários de sistema, caminhos e logging interno
│       ├── ApplicationPathUtils.hpp/.cpp # Resolução segura de caminhos da aplicação
│       ├── CaminhoPersistenteUtils.hpp/.cpp # Gerenciamento de histórico de caminhos persistentes
│       └── LogManager.hpp/.cpp         # Logging de diagnóstico interno da aplicação
├── CMakeLists.txt                 # Script mestre do CMake (Targets: MuninEngine, Munin, munin-app-cli, MuninBench, MuninDeploy)
├── CMakePresets.json              # Presets de configuração e build para MinGW e MSVC
├── CODE_OF_CONDUCT.md             # Código de Conduta para colaboradores e comunidade
├── CONTRIBUTING.md                # Diretrizes de contribuição, padrões de código e fluxo de PR
├── LICENSE                        # Licença de código aberto GNU AGPLv3 do projeto
├── main.cpp                       # Ponto de entrada alternativo/raiz do projeto
├── MANUAL.md                      # Manual completo de utilização e operação técnica
├── OBSERVACAO.md                  # Registro do estado técnico e arquitetural da versão
├── README.md                      # Documentação multilíngue oficial do repositório
└── SECURITY.md                    # Política de segurança, reporte de vulnerabilidades e modelo de ameaças
```

---

## 6. Documentação Técnica Modular

- 🏛️ [Arquitetura do Sistema](docs/architecture.md) — Design da engine, mapeamento de memória Win32, vetorização SIMD e Qt6.
- 🛡️ [Modelo de Segurança e Integridade](docs/security-model.md) — Filosofia local-first, integridade de memória virtual e prevenção ReDoS.
- 📋 [Conformidade e Governança](docs/compliance.md) — Mapeamentos NIST SP 800-92, PCI-DSS v4.0, ISO 27001, LGPD e GDPR.
- ⚖️ [Licenciamento e Autorização Comercial](docs/licensing.md) — Termos GNU AGPLv3, diretrizes Microsoft Store e autorização do autor.
- 📖 [Guia de Operações do Usuário](docs/user-guide.md) — Operações passo a passo, busca paralela, timeline e inspetor JSON.
- ⚙️ [Referência de Linha de Comando](docs/cli-reference.md) — Parâmetros de execução do Munin.exe, munin-app-cli, MuninBench e find_VersionEditor.
- 📦 [Notas de Versão v2026.11.6.0](docs/release-v2026.11.6.0.md) — Hashes forenses SHA-256, artefatos de instalação e changelog.
- 💡 [Sugestões de Evolução e Issues Abertas](docs/suggested-issues.md) — Backlog técnico e melhorias sugeridas pela comunidade.
- 📄 [Manual Completo de Instruções](MANUAL.md) & [Observações Técnicas](OBSERVACAO.md) — Documentação aprofundada de engenharia.

---

## 7. Licença e Autorização Comercial Expressa

Este projeto está licenciado sob os termos da **GNU Affero General Public License v3 (AGPLv3)**. Consulte o arquivo [LICENSE](LICENSE) para obter a íntegra dos termos e condições.

### 7.1. Declaração do Autor e Liberdade para Desenvolvedores
> **Declaração do Titular dos Direitos:**  
> Eu, **Fernando Nillsson Cidade**, autor e criador do **Munin App**, asseguro plena liberdade aos desenvolvedores sob os termos da **GNU AGPLv3**:
> 1. **Documentação em `source/assets/`:** Foi elaborada exclusivamente para comercialização e conformidade na Microsoft Store. O autor autoriza expressamente desenvolvedores e colaboradores a realizarem alterações, exclusões, adições e modificações gerais nesta documentação e no código associado, fornecendo total liberdade para adaptá-los desde que respeitem as diretrizes e legislação de seus países e plataformas.
> 2. **Código-Fonte Aberto (Licença GNU AGPLv3):** O código-fonte integral está disponibilizado publicamente sob a Licença GNU AGPLv3. Qualquer pessoa pode auditar, compilar, modificar, criar forks e gerar seus próprios pacotes livremente sob os termos copyleft da licença.
> 3. **Restrição Exclusiva sobre o Binário Oficial do Autor:** Apenas o binário pré-compilado e assinado oficialmente pelo autor e distribuído na Microsoft Store **não possui autorização** para comercialização ou revenda por terceiros, tampouco para engenharia reversa (prática que ademais é desnecessária, visto que o código-fonte integral já é aberto neste repositório).

### 7.2. Fundamentação Jurídica da Documentação em `source/assets/`
A presença do conjunto formal de documentação institucional e regulatória mantido no diretório `source/assets/` (abrangendo os módulos `ABOUT`, `CLC`, `EULA`, `NOTICES`, `PRIVACY_POLICY`, `LICENSES` e `COPYRIGHT`) fundamenta-se nas seguintes exigências regulatórias, operacionais e de mercado, operando em total viabilidade e segurança jurídica:

1. **Habilitação para Distribuição Comercial e Marketplaces:** Plataformas corporativas de software, lojas de aplicativos (como a [Microsoft Store](https://apps.microsoft.com/detail/9PP4GVHR5THC)) e ecossistemas de distribuição comercial B2B exigem a presença de um Contrato de Licença de Usuário Final (*EULA / Application Developer Agreement - ADA*) e de uma Política de Privacidade transparente para homologação, publicação, proteção e venda regular do produto.
2. **Conformidade Estrita com Leis de Privacidade (LGPD e GDPR):** A *Política de Privacidade* e o *EULA* estabelecem de forma irrefutável e auditável o compromisso da arquitetura *Local-First* do Munin App (operação 100% autônoma e offline, sem telemetria, sem coleta de dados cadastrais ou comportamentais e sem transmissão externa de informações de arquivos ou identidades), resguardando juridicamente empresas usuárias em conformidade com a Lei Geral de Proteção de Dados (Lei nº 13.709/2018) e o GDPR da União Europeia.
3. **Atribuição de Terceiros e Licenciamento Upstream (LGPL v3):** Os diretórios `NOTICES` e `LICENSES` cumprem rigorosamente os deveres de atribuição e as cláusulas das licenças de componentes integrados de terceiros (em especial o framework **Qt 6.11.1** sob GNU LGPL v3), garantindo integridade de propriedade intelectual e conformidade de governança corporativa.
4. **Harmonia Jurídica entre Código Aberto e Uso Comercial:** Os termos do EULA e do CLC regulam as condições operacionais, garantias e responsabilidades de suporte da aplicação compilada distribuída ao usuário final, operando em perfeita harmonia com as liberdades outorgadas pela GNU AGPLv3 ao código-fonte, conferindo segurança jurídica integral tanto à comunidade de software livre quanto aos clientes comerciais corporativos.

*Munin App • © 2026 Fernando Nillsson Cidade*

## 8. Autor

Fernando Nillsson Cidade<br>
Contato: `linceu_lighthouse@outlook.com`<br>
GitHub: [https://github.com/fernandoncidade](https://github.com/fernandoncidade)

## 9. Contribuição e Comunidade

> [!NOTE]
> **Sobre o Desenvolvimento e Contribuições Externas:**  
> Este projeto é concebido, desenvolvido e mantido primariamente como uma iniciativa pessoal e educacional pelo autor, que é iniciante e está em contínuo aprendizado em programação. O envio de sugestões, abertura de *Issues* e submissão de *Pull Requests (PRs)* são sempre bem-vindos! No entanto, ressalta-se que a análise de código, interação em tópicos técnicos e incorporação de contribuições ocorrem de forma voluntária e no próprio ritmo do autor. Caso um chamado ou PR demore para ser respondido ou não seja incorporado, não reaja com alarde ou frustração. A licença GNU AGPLv3 garante a você total liberdade para criar um *Fork* independente e evoluir suas próprias versões.

Contribuições são bem-vindas! Consulte nosso [Guia de Contribuição](CONTRIBUTING.md) e [Código de Conduta](CODE_OF_CONDUCT.md).

</details>

---

## <a id="enus"></a>English (US)

> **Note:** This repository refers to the official **v2026.11.6.0** release of the **Munin App** project. Support the project and get the Windows version through the official link: [Install via Microsoft Store](https://apps.microsoft.com/detail/9PP4GVHR5THC)

<details>
<summary>Click to expand / collapse</summary><br>

# Munin App — Massive Log Reader

Version: v2026.11.6.0<br>
Technical revision date: November 6, 2026<br>
Author: Fernando Nillsson Cidade<br>

## Summary

**Munin App** is a high-performance massive log file reader and analytical workbench for Windows 11, natively engineered in modern **C++20** with an interactive user interface built on **Qt 6**.

Designed specifically to eliminate the traditional bottleneck where conventional editors and viewers freeze, take several minutes to build line indexes, or run out of memory when opening files ranging from hundreds of megabytes to tens of gigabytes, Munin App employs a fully virtualized, zero-copy architecture from disk to display:
- **Win32 Memory-Mapped Files**: On-demand zero-copy paging managed directly by the Windows NT kernel memory subsystem (`CreateFileMappingW` and `MapViewOfFile`), opening 1 GB, 10 GB, or larger files in fractions of a second with negligible physical RAM usage;
- **Hardware SIMD Vectorized Indexing (AVX2 & SSE4.2)**: High-throughput newline scanning (`\n`) achieving throughput rates exceeding 3 GB/s;
- **Real-Time Live Tail Engine**: Continuous tracking of concurrently appended records using kernel-level asynchronous notifications (`ReadDirectoryChangesW`) without CPU polling loops;
- **Multi-Core Parallel Search**: Partitioned chunk-based multithreaded searching across all logical CPU cores, supporting JIT Regular Expressions, case-matching, and matched-line filter modes;
- **Temporal Volume Histogram (Time-Slice Scanner)**: Automatic ISO-8601 timestamp parsing paired with interactive density histograms to identify activity surges and error spikes immediately;
- **Structured JSON & Record Inspector**: Detailed syntax-highlighted hierarchy tree for modern structured JSON logs;
- **Adaptive Interface & Themes**: Native Windows 11 integration (Mica, DWM dark/light title bar) and runtime dynamic localization between English (United States) and Português (Brasil).

---

## Key Features

| Feature | Technical Description |
| :--- | :--- |
| **Instant Log Opening** | Opens massive log files in fractions of a second without buffer duplications, leveraging Win32 memory-mapped virtual paging. |
| **SIMD Line Indexing** | Vectorized 256-bit (AVX2) and 128-bit (SSE4.2) processing with scalar fallback to scan line delimiters at > 3 GB/s. |
| **Sparse Chunk Indexer** | 64 KB sparse block partitioning with offset lookups, enabling $O(1)$ random seek to any line across millions of records. |
| **Virtualized Log View** | Custom table view based on Qt6's `QAbstractItemView` and `QTableView`, only instantiating rows visible within the viewport. |
| **Density Minimap Scrollbar** | Interactive scrollbar displaying overall log distribution, search matches, and severity markers across the entire document. |
| **Live Tail Monitoring** | Real-time append tracking (`Ctrl+L`) with smooth auto-scrolling powered by native Windows file system notifications. |
| **Parallel Search Engine** | Multithreaded search (`Ctrl+F`) dividing chunks across CPU cores, with support for exact phrases, regex, and filter mode. |
| **Time-Slice Analysis** | Event density histogram (`Ctrl+T`) allowing one-click filtering of critical operational timeframes and anomalies. |
| **JSON Record Inspector** | Retractable panel (`Ctrl+J`) dissecting structured JSON log entries into an interactive tree with one-click copying. |
| **Synthetic Log Generator** | Built-in test generator to create realistic 100 MB, 500 MB, and 1 GB synthetic log files for load testing and benchmarks. |
| **Terminal CLI Utility** | Standalone command-line executable (`munin-app-cli.exe`) for metadata inspection, parallel search, and live log streaming via console. |
| **Windows 11 Theme Sync** | Automatic runtime synchronization with Windows 11 Dark and Light modes, complete with native DWM title bar styling. |
| **Runtime Retranslation** | Dynamic switching between English (US) and Portuguese (BR) without requiring an application restart. |
| **100% Offline & Private** | Strictly autonomous and local execution. No logs, terms, paths, or analytics are ever transmitted over the network. |

---

## 📸 User Interface Screenshots

<div align="center">
  <table>
    <tr>
      <td align="center">
        <b>Massive Log Viewer & Density Minimap</b><br>
        <img src="repo/LISTAGEM/en_US/Captura%20de%20tela%202026-10-17%20024044.png" width="400" alt="Log Viewer & Minimap" /><br>
        <i>Fluid virtual navigation with zero-copy memory mapping</i>
      </td>
      <td align="center">
        <b>Multi-Core Parallel Search & JIT Filtering</b><br>
        <img src="repo/LISTAGEM/en_US/Captura%20de%20tela%202026-10-17%20024121.png" width="400" alt="Multi-Core Parallel Search" /><br>
        <i>Rapid pattern matching with chromatic markers on the scrollbar</i>
      </td>
    </tr>
    <tr>
      <td align="center" colspan="2">
        <b>Time-Slice Timeline & Structured JSON Inspector</b><br>
        <img src="repo/LISTAGEM/en_US/Captura%20de%20tela%202026-10-17%20024141.png" width="600" alt="Time-Slice Timeline & JSON Inspector" /><br>
        <i>Incident surge detection and structured hierarchy dissection</i>
      </td>
    </tr>
  </table>
</div>

---

## 📚 Modular Technical Documentation

- 📐 **[System Architecture](docs/architecture.md):** Win32 zero-copy design, AVX2/SSE4.2 SIMD mechanics, and Qt 6 virtual models.
- 💻 **[Command Line Reference](docs/cli-reference.md):** CLI syntax, launch arguments, and `MuninBench` benchmark utility.
- 🛡️ **[Security Model & Threat Analysis](docs/security-model.md):** Read-only memory mapping, buffer overflow prevention, and 100% offline security.
- ⚖️ **[Compliance Mapping](docs/compliance.md):** NIST SP 800-92, PCI-DSS v4.0, ISO 27001, LGPD, and GDPR log governance.
- 📜 **[Licensing & Commercial Model](docs/licensing.md):** GNU AGPLv3 open-source license, Microsoft Store release, and complete developer freedom.
- 📖 **[User Guide & Operations Manual](docs/user-guide.md):** Step-by-step instructions for viewing, Live Tail, and structured analysis.
- 📦 **[Release Notes](docs/release-v2026.11.6.0.md):** Release highlights, SHA-256 hashes, and distribution artifacts.
- 💡 **[Starter Community Issues](docs/suggested-issues.md):** Curated task templates ready for contributors.
- 🤝 **[Contributing Guide](CONTRIBUTING.md)** | 📜 **[Code of Conduct](CODE_OF_CONDUCT.md)** | 🔒 **[Security Policy](SECURITY.md)**

---

## Technical Highlights of Version 2026.10.19.0

1. **Decoupled C++20 Core Engine (`MuninEngine`)**:
   - Clean architectural isolation of low-level I/O, SIMD algorithms, and search mechanics into an independent static library.
   - Zero dependency on GUI headers in the processing engine for maximum reusability and benchmark accuracy.

2. **SIMD Vector Acceleration (`SimdUtils.cpp`)**:
   - Runtime CPU feature detection ensuring AVX2 instructions are executed when supported, utilizing `__m256i` registers and byte masks (`_mm256_movemask_epi8`) to inspect 32 bytes per clock cycle.
   - Full compatibility with SSE4.2 hardware (16 bytes per cycle) and a highly optimized scalar fallback.

3. **Multi-Core Parallel Search (`ParallelSearch.cpp`)**:
   - Dynamic worker thread pool scaled to `std::thread::hardware_concurrency()`.
   - Work units partitioned into pre-allocated 64 KB chunks, minimizing inter-thread memory contention and lock wait states.

4. **Kernel-Level Live Tail (`LiveTailWatcher.cpp`)**:
   - Passive tracking of active writes using the Windows `ReadDirectoryChangesW` kernel API on a dedicated background thread.
   - Dynamic expansion of line offset arrays without needing to reindex previously mapped regions.

5. **Modeless Dialog Hierarchy and Z-Order Control (`GerenciadorDialogos.cpp`)**:
   - Top-level modeless dialog design (`SobreDialog` and `ManualDialog`) uncoupled from the main window's window-manager hierarchy.
   - Enables seamless Z-order layering and focus switching between dialogs and the main viewer on multi-monitor setups.

6. **Unified Synchronization via Version Editor (`mocks/find_VersionEditor.cpp`)**:
   - Automated validation and synchronous bumping of version strings and release dates across all codebases, legal documents, and installers.

7. **Standalone Command-Line Interface (`munin-app-cli`)**:
   - Independent terminal utility executing high-speed log processing directly from PowerShell, CMD, or Windows Terminal without loading the Qt GUI runtime;
   - Full support for inspection (`info`), SIMD-accelerated parallel query (`search`), zero-polling live streaming (`tail`), and benchmarks (`bench`);
   - Integrated with `AppExecutionAlias` in MSIX and Inno Setup installers for direct global execution from any directory.

---

## Essential Keyboard Shortcuts

| Shortcut | Action |
| :--- | :--- |
| `Ctrl+O` | Open a log file in the viewer |
| `Ctrl+F` | Toggle the quick search bar |
| `Ctrl+T` | Toggle the Time-Slice distribution timeline |
| `Ctrl+J` | Toggle the structured JSON inspector |
| `Ctrl+L` | Toggle Live Tail monitoring mode |
| `Ctrl+G` | Open the synthetic test log generator |
| `Esc` | Clear active search filters or close retractable panels |
| `F1` | Open the Instruction Manual |
| `Alt+A` | Open the "File" menu |
| `Alt+E` | Open the "View" menu |
| `Alt+C` | Open the "Settings" menu |
| `Alt+B` or `Alt+S` | Open the "About" menu |

---

## System Requirements

- **Operating System**: Windows 10 (64-bit) Version 21H2 or Windows 11 (64-bit, recommended).
- **Processor**: Intel Core i3/i5/i7/i9 or AMD Ryzen x64 with SSE4.2 support (AVX2 recommended for optimal SIMD indexing).
- **Memory (RAM)**: 4 GB minimum (memory-mapped architecture requires negligible RAM even when working with 10+ GB log files).
- **Disk Storage**: 150 MB free disk space for complete binary distribution.

---

## Build and Compilation

### Prerequisites
- **CMake**: version 3.21 or higher.
- **Ninja**: high-speed build generator.
- **Qt 6**: Qt 6.11.1 (Core, Gui, Widgets).
- **Supported Compilers**:
  - MinGW 64-bit (GCC 13.1.0+);
  - Microsoft Visual C++ (MSVC 2022 v143+).

### 1. Building with MinGW (Preset `host-qt6-mingw-release`)

```powershell
# Configure project using MinGW preset
cmake --preset host-qt6-mingw-release

# Compile and perform full runtime deployment
cmake --build --preset host-qt6-mingw-release --target MuninDeploy --parallel
```

Binaries and deployed dependencies are available at:
- `build/build_Munin_mingw/Munin.exe`
- Clean external distribution: `D:\MISCELANEAS\VSCode\Munin\Munin_mingw\`

### 2. Building with MSVC (Preset `host-qt6-msvc-release`)

```powershell
# Configure project using MSVC preset via runner script
cmd.exe /c "cmake\run_msvc.bat cmake --preset host-qt6-msvc-release"

# Compile and perform full runtime deployment
cmd.exe /c "cmake\run_msvc.bat cmake --build --preset host-qt6-msvc-release --target MuninDeploy --parallel"
```

Binaries and deployed dependencies are available at:
- `build/build_Munin_msvc/Munin.exe`
- Clean external distribution: `D:\MISCELANEAS\VSCode\Munin\Munin_msvc\`

---

## Repository Structure

```text
Munin_App_cpp/
├── .github/                       # GitHub Actions CI/CD workflows and repository templates
│   ├── workflows/build.yml        # Automated multi-toolchain build pipeline (MinGW and MSVC)
│   ├── ISSUE_TEMPLATE/            # Issue reporting templates for bug reports and feature requests
│   └── PULL_REQUEST_TEMPLATE.md   # Standard pull request submission template
├── docs/                          # Modular technical documentation and compliance specifications
│   ├── architecture.md            # Win32 zero-copy architecture, SIMD AVX2/SSE4.2, and Qt6
│   ├── cli-reference.md           # Complete CLI reference for munin-app-cli and MuninBench
│   ├── compliance.md              # Compliance mapping for NIST SP 800-92, LGPD, and GDPR
│   ├── licensing.md               # GNU AGPLv3 licensing model and Microsoft Store authorization
│   ├── release-v2026.11.6.0.md    # Technical release notes, SHA-256 hashes, and distribution artifacts
│   ├── security-model.md          # Security model, threat analysis, and integrity guarantees
│   ├── suggested-issues.md        # Structured community contribution backlog
│   └── user-guide.md              # Step-by-step user operational guide
├── repo/                          # Inno Setup installation scripts, MSIX manifest, and assets
│   └── LISTAGEM/                       # Official Microsoft Store screenshot assets (pt_BR/en_US)
├── mocks/                         # Development utilities and auxiliary mock tools
│   ├── CMakeLists.txt                  # Build configuration for auxiliary mock targets
│   ├── CompileTranslationsGUI.hpp/.cpp # Interactive GUI for compiling and validating .ts -> .qm catalogs
│   ├── CompileTranslationsTerminal.hpp/.cpp # Command-line batch translation compiler
│   ├── find_VersionEditor.hpp/.cpp     # Automated version synchronization and integrity checker
│   └── icons/                          # Application icon assets for developer utilities
├── source/                        # Primary application source tree
│   ├── main.cpp                   # Application entry point for Munin (GUI)
│   ├── cli/                       # Standalone Headless Command Line Interface (CLI)
│   │   └── main_cli.cpp           # Entry point for munin-app-cli.exe
│   ├── engine/                    # Low-level Win32 / SIMD processing engine
│   │   ├── MemoryMappedFile.hpp/.cpp   # Win32 Memory-Mapped Files (zero-copy)
│   │   ├── SimdUtils.hpp/.cpp          # AVX2/SSE4.2 vectorized line scanning
│   │   ├── ChunkIndex.hpp/.cpp         # Sparse 64 KB block indexer
│   │   ├── LiveTailWatcher.hpp/.cpp    # Real-time asynchronous append monitoring
│   │   ├── ParallelSearch.hpp/.cpp     # Multi-core multithreaded parallel search
│   │   ├── TimeSliceScanner.hpp/.cpp   # Timestamp extraction and histogram scanner
│   │   └── TestEngine.cpp              # MuninBench performance benchmark target
│   ├── model/                     # Virtualized data models
│   │   └── LogModel.hpp/.cpp           # On-demand QAbstractTableModel for Qt6
│   ├── ui/                        # Modern Qt6 user interface components
│   │   ├── MainWindow.hpp/.cpp         # Main window, docking splitters, and status bar
│   │   ├── LogView.hpp/.cpp            # Customized virtualized table viewer
│   │   ├── LogItemDelegate.hpp/.cpp    # Custom item delegate for syntax highlighting and cell rendering
│   │   ├── MinimapScrollBar.hpp/.cpp   # Density minimap vertical scrollbar
│   │   ├── SearchBarWidget.hpp/.cpp    # Search bar widget with regex and occurrence counts
│   │   ├── TimeSliceWidget.hpp/.cpp    # Interactive temporal histogram widget
│   │   ├── JsonInspectorWidget.hpp/.cpp# Structured JSON inspector and tree viewer
│   │   ├── LogGeneratorDialog.hpp/.cpp # Synthetic benchmark log generator dialog
│   │   ├── StickyMenu.hpp/.cpp         # Fluent sticky menu bar implementation
│   │   ├── ThemeManager.hpp/.cpp       # Windows 11 Light/Dark theme synchronization
│   │   ├── Win32DwmHelper.hpp/.cpp     # Windows DWM integration (Mica & dark title bars)
│   │   ├── GerenciadorDialogos.hpp/.cpp# Modeless dialog manager (Z-Order control)
│   │   ├── SobreDialog.hpp/.cpp        # About dialog with historical notes and licenses
│   │   ├── OpcoesSobre.hpp/.cpp        # Advanced options selector for the About dialog
│   │   ├── ButtonUtils.hpp/.cpp        # UI button styling and standardization helpers
│   │   └── Manual.hpp/.cpp             # Interactive Instruction Manual dialog
│   ├── language/                  # Translation catalogs and dynamic locale manager
│   │   ├── GerenciadorTraducao.hpp/.cpp# Dynamic .qm translation catalog loader
│   │   └── translations/               # Translation files (munin_pt_BR.ts/.qm, munin_en_US.ts/.qm)
│   ├── assets/                    # Informational texts, EULAs, notices, policies, and icons
│   │   ├── ABOUT/                      # About text and architectural history
│   │   ├── CLC/                        # Corporate Commercial License Contracts (CLC)
│   │   ├── COPYRIGHT/                  # Formal copyright and trademark notices
│   │   ├── EULA/                       # End User License Agreements in PT-BR and EN-US
│   │   ├── LICENSES/                   # Full text for upstream and third-party open-source licenses
│   │   ├── NOTICES/                    # Third-party notices and open-source attributions
│   │   ├── PRIVACY_POLICY/             # Privacy policies conforming to LGPD and GDPR
│   │   ├── RELEASE/                    # Technical release notes and launch history
│   │   └── icons/                      # Multi-resolution icons in .ico and .png formats
│   └── utils/                     # System utilities, persistence, and internal logging
│       ├── ApplicationPathUtils.hpp/.cpp # Safe application and portable path resolution
│       ├── CaminhoPersistenteUtils.hpp/.cpp # Persistent path history and recent file manager
│       └── LogManager.hpp/.cpp         # Internal application diagnostics logging
├── CMakeLists.txt                 # Master CMake configuration (MuninEngine, Munin, munin-app-cli, MuninBench, MuninDeploy)
├── CMakePresets.json              # Configuration and build presets for MinGW and MSVC
├── CODE_OF_CONDUCT.md             # Code of Conduct for contributors and community
├── CONTRIBUTING.md                # Contribution guidelines, coding standards, and PR workflows
├── LICENSE                        # Repository GNU AGPLv3 open-source license
├── main.cpp                       # Alternative/root application entry point
├── MANUAL.md                      # Comprehensive user manual and operational guide
├── OBSERVACAO.md                  # Detailed technical architecture and snapshot notes
├── README.md                      # Official multilanguage repository documentation
└── SECURITY.md                    # Security policy, vulnerability reporting, and threat model
```

---

## 6. Modular Technical Documentation

- 🏛️ [System Architecture](docs/architecture.md) — Engine design, Win32 memory-mapped files, SIMD vectorization, and Qt6.
- 🛡️ [Security & Integrity Model](docs/security-model.md) — Local-first philosophy, virtual memory integrity, and anti-ReDoS.
- 📋 [Compliance & Governance](docs/compliance.md) — NIST SP 800-92, PCI-DSS v4.0, ISO 27001, LGPD, and GDPR mappings.
- ⚖️ [Licensing & Commercial Clarification](docs/licensing.md) — GNU AGPLv3 terms, Microsoft Store guidelines, and author's rights grant.
- 📖 [User Operations Guide](docs/user-guide.md) — Step-by-step operations, parallel search, timeline, and JSON inspector.
- ⚙️ [CLI Reference](docs/cli-reference.md) — Command-line syntax for Munin.exe, munin-app-cli, MuninBench, and find_VersionEditor.
- 📦 [Release Notes v2026.11.6.0](docs/release-v2026.11.6.0.md) — Forensic SHA-256 hashes, release artifacts, and changelog.
- 💡 [Evolution Roadmap & Suggested Issues](docs/suggested-issues.md) — Technical backlog and community contributions.
- 📄 [Comprehensive Manual](MANUAL.md) & [Technical Observations](OBSERVACAO.md) — In-depth engineering references.

---

## 7. License and Express Commercial Authorization

This project is licensed under the terms of the **GNU Affero General Public License v3 (AGPLv3)**. Refer to [LICENSE](LICENSE) for the complete terms and conditions.

### 7.1. Author's Declaration and Developer Freedom
> **Rights Holder Statement:**  
> I, **Fernando Nillsson Cidade**, author and creator of **Munin App**, guarantee full freedom to developers under the terms of the **GNU AGPLv3**:
> 1. **Documentation under `source/assets/`:** Authored exclusively for Microsoft Store commercial compliance and marketplace onboarding. The author expressly authorizes developers and contributors to modify, delete, add, and adapt this documentation and source code with complete freedom, provided they respect local laws and the regulations of their target publishing platforms.
> 2. **Open-Source Codebase (GNU AGPLv3 License):** The full source code is publicly available under the GNU AGPLv3. Anyone is free to audit, compile, modify, build custom executables, and maintain independent forks under the copyleft terms of the license.
> 3. **Exclusive Restriction on the Author's Official Binary:** Only the precompiled, digitally signed binary package provided directly by the author on the Microsoft Store **is not authorized** for resale or commercial redistribution by third parties, nor for reverse engineering (which is unnecessary anyway, since the author openly provides the unabridged source code in this repository).

### 7.2. Legal Justification for Documentation in `source/assets/`
The formal collection of institutional and regulatory documentation maintained under the `source/assets/` directory (encompassing `ABOUT`, `CLC`, `EULA`, `NOTICES`, `PRIVACY_POLICY`, `LICENSES`, and `COPYRIGHT`) is grounded in the following legal, operational, and commercial distribution mandates:

1. **Commercial Distribution & Enterprise Marketplaces:** Enterprise app stores (such as the [Microsoft Store](https://apps.microsoft.com/detail/9PP4GVHR5THC)), enterprise application catalogs, and commercial B2B procurement channels strictly mandate the inclusion of formal End User License Agreements (*EULA / Application Developer Agreement - ADA*) and clear Privacy Policies for product onboarding, certification, and commercial billing.
2. **Data Privacy and Protection Compliance (GDPR & LGPD):** The *Privacy Policy* and *EULA* provide auditable, legally binding guarantees that Munin App's *Local-First* architecture operates strictly offline (zero telemetry, zero data collection, no background tracking, and no external server communications), safeguarding enterprise users and ensuring compliance with the European Union GDPR and Brazilian LGPD (Federal Law No. 13,709/2018).
3. **Upstream Licensing Compliance & Third-Party Attribution (LGPL v3):** The `NOTICES` and `LICENSES` folders meticulously fulfill all attribution and compliance obligations required by integrated third-party libraries (notably the **Qt 6.11.1** framework under GNU LGPL v3), ensuring complete intellectual property hygiene.
4. **Legal Harmony Between Open-Source Code and Commercial Delivery:** The EULA and CLC establish the terms of distribution, warranty limitations, and operational usage for the compiled and packaged end-user binary application without constraining or diminishing the perpetual rights granted under the GNU AGPLv3 for the source code, creating a seamless and secure legal environment for both open-source contributors and commercial customers.

*Munin App • © 2026 Fernando Nillsson Cidade*

## 8. Author

Fernando Nillsson Cidade<br>
Contact: `linceu_lighthouse@outlook.com`<br>
GitHub: [https://github.com/fernandoncidade](https://github.com/fernandoncidade)

## 9. Contributing & Community

> [!NOTE]
> **About Project Development, Support, and External Contributions:**  
> This project is designed, developed, and maintained primarily as a personal and educational initiative by the author, who is a beginner and in continuous learning in software engineering. Suggestions, feedback, *Issues*, and *Pull Requests (PRs)* are always welcomed from the community! However, please note that code reviews, technical discussions, and upstream merges are conducted voluntarily at the author's own pace. If a ticket or PR is not answered promptly or is not merged into the mainline branch, please do not react with alarm or frustration. The GNU AGPLv3 license guarantees complete freedom to create an independent *Fork* and evolve your own customized builds.

Contributions are welcome! Please review our [Contributing Guide](CONTRIBUTING.md) and [Code of Conduct](CODE_OF_CONDUCT.md).

</details>
