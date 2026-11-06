# Modelo de Segurança / Security Model & Threat Analysis — Munin App

<p align="center">
  <b>Selecione o idioma / Select language:</b><br>
  <a href="#ptbr">🇧🇷 Português (BR)</a> | <a href="#enus">🇺🇸 English (US)</a>
</p>

---

## <a id="ptbr"></a>🇧🇷 Português (BR)

<details>
<summary>Clique para expandir / recolher o modelo de segurança em português</summary><br>

# Munin App — Modelo de Segurança e Análise de Ameaças

**Versão:** `2026.11.6.0`  
**Data:** `6 de novembro de 2026`  
**Nível de Segurança:** 100% Local-First | Zero Telemetria | Abertura Somente Leitura  
**Contexto Operacional:** Windows 11 / Windows 10 (x64)  
**Autor:** Fernando Nillsson Cidade  

---

## 1. Modelo de Ameaças e Garantias de Design

O **Munin App** foi construído para operar com segurança em ambientes corporativos de alta criticidade (SRE, SOC, DevOps, infraestrutura financeira e governamental).

### 1.1. Garantias Fundamentais de Segurança
1. **Comunicação de Rede Nula (Zero Network):**
   - A aplicação não possui componentes de rede, sockets TCP/UDP, servidores HTTP ou clientes web.
   - Inexistência total de código de telemetria, relatórios de uso ou métricas externas.
2. **Operação Estritamente Somente Leitura:**
   - Chamadas de abertura via `CreateFileW` utilizam exclusivamente o privilégio `GENERIC_READ`.
   - Os arquivos são abertos com compartilhamento completo (`FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE`), garantindo que servidores em produção possam continuar gravando seus logs sem bloqueios ou corrupção provocados pelo visualizador.
3. **Resistência a Esgotamento de Memória (Anti-DoS):**
   - Ao contrário de editores tradicionais que alocam gigabytes de objetos `QString` ou buffers de texto na memória heap, o Munin App utiliza mapeamento virtual de páginas Win32 sob demanda. O consumo de RAM permanece previsível e reduzido (< 60 MB), impedindo que logs gigantescos derrubem a estação de trabalho.

---

## 2. Segurança de Memória e Vetorização SIMD em C++20

### 2.1. Delimitação Estrita de Blocos em Registradores Vetoriais
A aceleração vetorial via instruções AVX2 (`_mm256_movemask_epi8`) e SSE4.2 (`_mm_movemask_epi8`) processa blocos de 32 e 16 bytes:
- **Proteção de Borda:** Antes de cada instrução vetorial, o algoritmo valida se a distância até o final do mapeamento é suficiente para carregar um vetor completo.
- **Cauda Escalar Segura:** Os bytes remanescentes que não completam um bloco de 32 bytes são processados por uma rotina escalar clássica de byte a byte, eliminando qualquer risco de leitura fora dos limites alocados (*out-of-bounds read*).

### 2.2. Prevenção de Falhas de Expressões Regulares (ReDoS)
- O motor de busca com Expressões Regulares utiliza compilação JIT de alta performance com limites estritos de retrocesso (*backtracking*), prevenindo ataques de negação de serviço algorítmica por regex patológico.

---

## 3. Persistência e Privilégios Mínimos

- **Execução Sem Privilégios de Administrador:** O Munin App opera plenamente no contexto de usuário padrão. Não requer elevação UAC para inspecionar logs pertencentes ao usuário ou compartilhamentos com permissão de leitura.
- **Armazenamento Seguro de Configurações:** Preferências de interface e histórico de arquivos recentes são mantidos exclusivamente no diretório local do perfil do usuário em `%LOCALAPPDATA%\Munin App`, sem escrita em chaves globais sensíveis do Registro do Windows.

</details>

---

## <a id="enus"></a>🇺🇸 English (US)

<details>
<summary>Click to expand / collapse security model in English</summary><br>

# Munin App — Security Model and Threat Analysis

**Version:** `2026.11.6.0`  
**Date:** `November 6, 2026`  
**Security Profile:** 100% Local-First | Zero Telemetry | Read-Only File Mapping  
**Target Environment:** Windows 11 / Windows 10 (x64)  
**Author:** Fernando Nillsson Cidade  

---

## 1. Threat Model & Design Guarantees

**Munin App** is designed for secure deployment within mission-critical enterprise environments (SRE, SOC, DevOps, financial, and governmental infrastructure).

### 1.1. Core Security Guarantees
1. **Zero Network Communication:**
   - No network sockets, HTTP clients, or external web services exist in the binary.
   - Zero telemetry, analytics, or remote tracking.
2. **Strict Read-Only Operation:**
   - All file operations invoke `CreateFileW` strictly with `GENERIC_READ`.
   - Opened with non-exclusive sharing (`FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE`), guaranteeing active production daemons continue logging without lock contention or corruption.
3. **Denial-of-Service (DoS) Resistance:**
   - Unlike conventional text editors that inflate heap memory, Munin App demand-maps virtual pages. RAM footprint remains low (< 60 MB), preventing system crashes caused by multi-gigabyte log inspection.

---

## 2. Memory Safety & SIMD Vectorization in C++20

### 2.1. Strict Boundary Checks in Vector Registers
SIMD vectorization via AVX2 (`_mm256_movemask_epi8`) and SSE4.2 (`_mm_movemask_epi8`) inspects 32-byte and 16-byte chunks:
- **Boundary Clamping:** Prior to issuing vector loads, remaining mapped size is strictly checked.
- **Safe Scalar Tail:** Remaining trailing bytes that do not fill a complete 32-byte vector are parsed byte-by-byte, eliminating out-of-bounds reads.

### 2.2. Regular Expression DoS (ReDoS) Mitigation
- The regular expression search engine leverages JIT compilation with bounded backtracking to prevent algorithmic freezes on pathological regex patterns.

---

## 3. Least Privilege & Local Persistence

- **Unprivileged Execution:** Munin App runs completely within standard user privileges without requiring UAC elevation.
- **Local Profile Persistence:** Application settings and recent file lists are stored strictly within `%LOCALAPPDATA%\Munin App`, avoiding sensitive system-wide registry mutations.

</details>
