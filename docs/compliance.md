# Mapeamento de Conformidade / Compliance Mapping — Munin App

<p align="center">
  <b>Selecione o idioma / Select language:</b><br>
  <a href="#ptbr">🇧🇷 Português (BR)</a> | <a href="#enus">🇺🇸 English (US)</a>
</p>

---

## <a id="ptbr"></a>🇧🇷 Português (BR)

<details>
<summary>Clique para expandir / recolher o guia de conformidade em português</summary><br>

# Munin App — Mapeamento de Conformidade Regulatória e Governança de Logs

**Versão:** `2026.11.6.0`  
**Data:** `6 de novembro de 2026`  
**Frameworks:** NIST SP 800-92 | PCI-DSS v4.0 | ISO/IEC 27001 | LGPD / GDPR | SOC 2  

---

## 1. Visão Geral de Conformidade

O gerenciamento, análise e custódia de registros de log constituem uma obrigação legal e regulatória indispensável em ambientes corporativos e governamentais. O **Munin App** foi projetado especificamente para atender a essas exigências, assegurando análise forense de alto desempenho com integridade estrita e soberania absoluta de dados.

---

## 2. Mapeamento de Padrões e Normas

### 2.1. NIST SP 800-92 (Guide to Computer Security Log Management)
- **Seção 4 — Arquitetura de Gerenciamento de Logs:**
  - O NIST estabelece que ferramentas de análise de logs devem viabilizar inspeções rápidas sem comprometer o armazenamento de origem.
  - *Atendimento pelo Munin App:* Operação estritamente em modo somente leitura (`GENERIC_READ` com `FILE_SHARE_READ | FILE_SHARE_WRITE`), garantindo que trilhas de auditoria jamais sejam corrompidas, truncadas ou alteradas.

### 2.2. PCI-DSS v4.0 (Payment Card Industry Data Security Standard)
- **Requisito 10 — Registrar e Monitorar Todos os Acessos aos Componentes do Sistema:**
  - Exige a revisão periódica de registros de log de servidores e componentes que processem dados de cartões de pagamento.
  - *Atendimento pelo Munin App:* Abertura instantânea de gigabytes de logs de transações e servidores web (IIS, Nginx, Apache) sem estouro de memória, viabilizando auditorias rápidas e localização de anomalias com filtros de tempo (Time-Slice).

### 2.3. ISO/IEC 27001:2022
- **Controle A.8.15 — Registro de Logs (Logging):**
  - Logs registrando eventos de segurança, falhas e exceções devem ser mantidos e analisados para proteção contra ataques.
  - *Atendimento pelo Munin App:* Busca paralela multi-core com Expressões Regulares (Regex JIT) para identificar instantaneamente tentativas de injeção, acessos negados repetidos e comportamentos suspeitos.

### 2.4. LGPD (Lei nº 13.709/2018) & GDPR (Regulamento UE 2016/679)
- **LGPD Art. 46 / GDPR Artigo 32 — Segurança do Tratamento de Dados:**
  - Exige medidas de segurança técnicas e administrativas aptas a proteger os dados pessoais contra acessos não autorizados.
  - *Atendimento pelo Munin App:* Arquitetura **100% Local-First e Offline**. O Munin App não possui componentes de rede, não transmite dados para servidores em nuvem, não coleta telemetria e mantém os dados de log confidenciais confinados estritamente à máquina do analista.

---

## 3. Matriz de Controles de Governança

| Requisito Regulatório | Mecanismo Implementado no Munin App | Evidência Técnica |
| :--- | :--- | :--- |
| **Integridade da Evidência** | Abertura sem escrita com chamadas nativas Win32. | Chamadas `CreateFileW` restritas a `GENERIC_READ`. |
| **Confidencialidade Estrita** | Operação sem tráfego de rede ou armazenamento externo. | Nenhuma chamada de socket ou requisição HTTP. |
| **Inspeção de Logs Estruturados** | Formatação e dissecção de payloads JSON sem envio externo. | `JsonInspectorWidget` com parser estritamente local. |
| **Resposta Rápida a Incidentes** | Varredura de centenas de gigabytes a >3 GB/s com SIMD. | `SimdUtils` com vetorização AVX2/SSE4.2 e `ParallelSearch`. |

---

### ⚠️ Nota de Isenção Regulatória (Regulatory Disclaimer)
O **Munin App** é uma ferramenta utilitária e educacional de análise e visualização de arquivos de log. A utilização do software não garante por si só a conformidade legal ou regulatória com nenhuma das normas mencionadas (NIST, PCI-DSS, ISO 27001, LGPD, GDPR, SOC 2). A responsabilidade final pela governança, custódia e conformidade regulatória perante autoridades e órgãos certificadores é exclusiva da organização que o opera.

</details>

---

## <a id="enus"></a>🇺🇸 English (US)

<details>
<summary>Click to expand / collapse compliance guide in English</summary><br>

# Munin App — Regulatory Compliance & Log Governance Mapping

**Version:** `2026.11.6.0`  
**Date:** `November 6, 2026`  
**Frameworks:** NIST SP 800-92 | PCI-DSS v4.0 | ISO/IEC 27001 | LGPD / GDPR | SOC 2  

---

## 1. Compliance Overview

Audit trail analysis, custody, and log governance are critical mandates across regulatory frameworks. **Munin App** is specifically engineered to meet these standards, providing forensic log analysis while ensuring zero tampering and complete data sovereignty.

---

## 2. Regulatory Standard Mapping

### 2.1. NIST SP 800-92 (Guide to Computer Security Log Management)
- **Section 4 — Log Management Architecture:**
  - Mandates rapid inspection tools that preserve evidence without altering source logs.
  - *Munin App Implementation:* Strict read-only file mapping (`GENERIC_READ` with `FILE_SHARE_READ | FILE_SHARE_WRITE`), ensuring source audit trails are never truncated, overwritten, or modified.

### 2.2. PCI-DSS v4.0 (Payment Card Industry Data Security Standard)
- **Requirement 10 — Log and Monitor All Access to System Components:**
  - Requires regular reviews of audit logs for servers processing cardholder data.
  - *Munin App Implementation:* Instantaneous opening of multi-gigabyte transaction logs (IIS, Nginx, Apache) without memory exhaustion, facilitating prompt investigation via time-slice histograms.

### 2.3. ISO/IEC 27001:2022
- **Control A.8.15 — Logging:**
  - Event logs recording anomalies, security exceptions, and faults must be maintained and analyzed.
  - *Munin App Implementation:* Parallel multi-core regular expression searching (PCRE2 JIT) to rapidly detect injection attempts, brute force errors, and operational anomalies.

### 2.4. LGPD (Law 13,709/2018) & GDPR (EU Regulation 2016/679)
- **LGPD Art. 46 / GDPR Article 32 — Security of Processing:**
  - Mandates technical measures to protect sensitive data from unauthorized disclosure.
  - *Munin App Implementation:* **100% Local-First & Offline** architecture. No network sockets, zero telemetry, and zero data leakage to cloud servers.

---

## 3. Governance Controls Matrix

| Regulatory Requirement | Munin App Implementation | Technical Evidence |
| :--- | :--- | :--- |
| **Evidence Integrity** | Non-destructive read-only file mapping. | Native `CreateFileW` calls restricted to `GENERIC_READ`. |
| **Strict Confidentiality** | Autonomous local execution with no cloud transmission. | Complete absence of socket libraries or external web hooks. |
| **Structured Log Review** | Local parsing and formatting of structured JSON logs. | `JsonInspectorWidget` running strictly in-process. |
| **Rapid Incident Triage** | Vectorized scanning exceeding 3 GB/s via SIMD. | `SimdUtils` AVX2/SSE4.2 vectorization and `ParallelSearch`. |

---

### ⚠️ Regulatory Disclaimer
**Munin App** is a technical software utility for log inspection, performance diagnostics, and forensic review support. Its use does not in itself grant automated compliance certification, nor does it substitute for formal internal policies, organizational governance processes, or security controls required by regulatory authorities, legislation, or accredited certification bodies (NIST, PCI-DSS, ISO 27001, LGPD, GDPR, SOC 2).

</details>
