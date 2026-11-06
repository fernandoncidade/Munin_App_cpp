# Política de Segurança / Security Policy — Munin App

<p align="center">
  <b>Selecione o idioma / Select language:</b><br>
  <a href="#ptbr">🇧🇷 Português (BR)</a> | <a href="#enus">🇺🇸 English (US)</a>
</p>

---

## <a id="ptbr"></a>🇧🇷 Português (BR)

<details>
<summary>Clique para expandir / recolher a política em português</summary><br>

# Política de Segurança

## 1. Versões com Suporte Oficial

Atualizações e correções de segurança são fornecidas ativamente para as versões oficiais:

| Versão | Suportada |
|:---:|:---:|
| `v2026.10.x` | :white_check_mark: |
| `< v2026.10` | :x: |

---

## 2. Reportando uma Vulnerabilidade

Levamos a estabilidade e a segurança do **Munin App** com máxima seriedade.

Se você identificou uma potencial vulnerabilidade de segurança (como falha em tratamento de limites de memória virtual mapeada, condição de corrida em leitura concorrente ou comportamento anômalo em processamento de expressões regulares):

1. **NÃO divulgue publicamente a vulnerabilidade através de issues ou fóruns abertos.**
2. Submeta o reporte de forma confidencial através da aba [Security](../../security/advisories/new) do repositório ou contate o mantenedor diretamente por e-mail: **linceu_lighthouse@outlook.com**.
3. Por favor, inclua no seu relato:
   - Descrição detalhada da anomalia;
   - Passo a passo para reprodução, acompanhado de arquivo de log ou amostra sintética;
   - Arquitetura de processador utilizada (instruções AVX2, SSE4.2 ou modo escalar);
   - Vetor de exploração e impacto potencial estimado;
   - Proposta de correção ou patch, se disponível.

### Prazos e Processo de Resposta
- O mantenedor envidará os melhores esforços para analisar e confirmar o recebimento de reportes de vulnerabilidade em tempo hábil, considerando a natureza estritamente voluntária e educacional da manutenção do projeto.
- Conduziremos análise técnica aprofundada na engine C++20 Win32.
- Uma vez sanada, lançaremos uma atualização oficial com os devidos créditos de divulgação responsável.

---

## 3. Compromissos Arquiteturais de Segurança

- **100% Autônomo, Local e Offline:** O Munin App não realiza nenhuma comunicação de rede, telemetria ou envio de dados para servidores externos.
- **Abertura em Modo Estritamente Somente Leitura:** Os arquivos de log são mapeados utilizando `GENERIC_READ` com flags de compartilhamento aberto (`FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE`), garantindo que o arquivo nunca seja corrompido ou alterado pelo visualizador.
- **Segurança de Memória Virtual:** Delimitação de páginas protegida via manipulação segura de ponteiros na API Win32 (`MapViewOfFile` / `UnmapViewOfFile`).

</details>

---

## <a id="enus"></a>🇺🇸 English (US)

<details>
<summary>Click to expand / collapse policy in English</summary><br>

# Security Policy

## 1. Supported Versions

Security updates and patches are actively provided for current official releases:

| Version | Supported |
|:---:|:---:|
| `v2026.10.x` | :white_check_mark: |
| `< v2026.10` | :x: |

---

## 2. Reporting a Vulnerability

We treat the stability, integrity, and safety of **Munin App** with the highest priority.

If you discover a potential security vulnerability (such as an out-of-bounds read in mapped virtual memory, SIMD boundary issues, or unexpected behavior in regular expression parsing):

1. **Do NOT disclose the issue publicly through open issues or discussions.**
2. Submit a confidential report via the repository's [Security Advisories](../../security/advisories/new) tab or contact the maintainer directly via email: **linceu_lighthouse@outlook.com**.
3. Please include:
   - A clear description of the issue;
   - Step-by-step reproduction instructions with sample log lines or synthetic data;
   - Hardware/CPU architecture details (AVX2, SSE4.2, or scalar fallback);
   - Potential impact and attack vector;
   - Proposed mitigation or fix, if available.

### Response Timeline
- The maintainer will make best efforts to acknowledge receipt and evaluate vulnerability reports in a timely manner, given the voluntary and educational nature of project maintenance.
- We will perform in-depth analysis on the C++20 Win32 engine.
- Upon resolution, a security patch will be released with appropriate credits for coordinated disclosure.

---

## 3. Architectural Security Guarantees

- **100% Local-First & Offline:** Munin App does not communicate over the network, collect telemetry, or transmit log contents to external servers.
- **Strictly Read-Only File Mapping:** Files are opened with `GENERIC_READ` and non-exclusive sharing (`FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE`), ensuring examined log files are never mutated or corrupted.
- **Memory Safety:** Boundary-checked virtual memory page views managed via native Win32 `MapViewOfFile` and RAII wrappers.

</details>
