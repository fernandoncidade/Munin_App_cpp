# Licenciamento e Esclarecimento Comercial / Licensing & Commercial Model — Munin App

<p align="center">
  <b>Selecione o idioma / Select language:</b><br>
  <a href="#ptbr">🇧🇷 Português (BR)</a> | <a href="#enus">🇺🇸 English (US)</a>
</p>

---

## <a id="ptbr"></a>🇧🇷 Português (BR)

<details>
<summary>Clique para expandir / recolher o documento de licenciamento em português</summary><br>

# Munin App — Licenciamento, Termos de Código Aberto e Esclarecimento Comercial

**Versão:** `2026.11.6.0`  
**Data:** `6 de novembro de 2026`  
**Licença do Código-Fonte:** GNU Affero General Public License v3 (AGPLv3)  
**Autor e Detentor dos Direitos Autorais:** Fernando Nillsson Cidade  

---

## 1. Visão Geral do Modelo de Licenciamento

O **Munin App — Leitor de Logs Massivos** adota um modelo aberto transparente que concilia a liberdade do ecossistema de software livre copyleft com a distribuição corporativa oficial:

| Componente | Licença | Usos Permitidos |
|---|---|---|
| **Código-Fonte (Repositório Completo)** | [GNU AGPLv3](../LICENSE) | 100% Livre: Uso, estudo, modificação, distribuição e contribuição sob termos copyleft da AGPLv3. |
| **Documentação em `source/assets/`** | Autoria do Projeto (Uso Livre para Desenvolvedores) | Criada exclusivamente para atender às exigências da Microsoft Store; desenvolvedores possuem liberdade total para alterar, excluir, adicionar e adaptar em seus próprios forks e plataformas. |
| **Binários Compilados por Terceiros (via Código)** | GNU AGPLv3 | Totalmente livres para compilar, executar, empacotar e redistribuir sob a responsabilidade do desenvolvedor, cumprindo os termos da AGPLv3. |
| **Binário Oficial Compilado na Microsoft Store** | Edição Oficial do Autor | Proibida a revenda, comercialização de terceiros ou engenharia reversa do binário oficial fornecido pelo autor. |
| **Bibliotecas de Terceiros (Qt 6, etc.)** | LGPL v3 / Licenças Compatíveis | Cumprem integralmente os requisitos de atribuição de código aberto upstream. |

---

## 2. Direitos de Código Aberto (GNU AGPLv3)

Todo o código-fonte do Munin App — abrangendo a engine de baixo nível em C++20 (`MuninEngine`), o utilitário de benchmark (`MuninBench`), o utilitário standalone de linha de comando (`munin-app-cli`), os modelos virtuais Qt 6, os componentes de interface (`LogView`, `MinimapScrollBar`, `TimeSliceWidget`, `JsonInspectorWidget`), os presets do CMake e os mocks de auditoria — está licenciado sob a **GNU Affero General Public License v3 (AGPLv3)**:

```text
Munin App — Massive Log Viewer for Windows
Copyright (c) 2025-2026 Fernando Nillsson Cidade

Este programa é um software livre: você pode redistribuí-lo e/ou modificá-lo sob os
termos da GNU Affero General Public License conforme publicada pela Free Software
Foundation, seja a versão 3 da Licença, ou (a seu critério) qualquer versão posterior.

Este programa é distribuído na esperança de que seja útil, mas SEM QUALQUER GARANTIA;
sem mesmo a garantia implícita de COMERCIALIZAÇÃO ou ADEQUAÇÃO A UM DETERMINADO FIM.
Veja a GNU Affero General Public License para mais detalhes.

Você deve ter recebido uma cópia da GNU Affero General Public License junto com este
programa. Se não, veja em: <https://www.gnu.org/licenses/>.
```

---

## 3. Documentação em `source/assets/` e Liberdade Total para Desenvolvedores

A documentação institucional e regulatória mantida no diretório `source/assets/` (abrangendo os módulos `ABOUT`, `CLC`, `COPYRIGHT`, `EULA`, `LICENSES`, `NOTICES`, `PRIVACY_POLICY` e `RELEASE`) possui status e finalidade específicos:

1. **Finalidade na Microsoft Store:** Esta documentação foi elaborada exclusivamente para atender às exigências legais, regulatórias e operacionais de publicação da aplicação empacotada na **Microsoft Store** (cumprindo requisitos de EULA, ADA, políticas de privacidade e governança LGPD/GDPR para marketplaces).
2. **Autorização Expressa do Autor para Desenvolvedores:** O autor autoriza expressamente todos os desenvolvedores, mantenedores de forks e colaboradores a realizarem quaisquer alterações, exclusões, acréscimos ou adaptações completas nesta documentação e no código associado. Você possui total liberdade para ajustar, remover termos de Store, reescrever manuais ou renomear módulos, desde que respeite as diretrizes e leis vigentes no seu país e na plataforma em que for publicar.
3. **Condição de Responsabilidade:** Os desenvolvedores são livres para adaptar ou remover esses textos em seus próprios forks e distribuições, desde que respeitem as diretrizes, normas e a legislação vigente de seus respectivos países e das plataformas em que decidirem publicar suas próprias compilações.

---

## 4. Diferenciação dos Binários Oficiais Compilados pelo Autor

O projeto opera sob o modelo de **Duplo Licenciamento** pragmático:
- **Código Aberto no GitHub:** O código-fonte integral é livre e aberto sob a GNU AGPLv3. Qualquer desenvolvedor pode auditar, compilar, construir seus próprios instaladores e redistribuí-los sob as obrigações da AGPLv3.
- **Binário Oficial Pré-compilado na Microsoft Store:** Distribuído como pacote assinado digitalmente para usuários e organizações que buscam conveniência, instalação automatizada sem privilégios de compilação e suporte via canal oficial. Os termos de EULA comercial aplicam-se unicamente ao pacote binário assinado distribuído pela Microsoft Store.

</details>

---

## <a id="enus"></a>🇺🇸 English (US)

<details>
<summary>Click to expand / collapse licensing terms in English</summary><br>

# Munin App — Licensing, Open Source Terms & Commercial Clarification

**Version:** `2026.11.6.0`  
**Date:** `November 6, 2026`  
**Source Code License:** GNU Affero General Public License v3 (AGPLv3)  
**Author & Copyright Holder:** Fernando Nillsson Cidade  

---

## 1. Licensing Model Overview

**Munin App — Massive Log Viewer** implements an open, transparent, and developer-friendly licensing model, reconciling copyleft open source freedom with official turnkey distribution:

| Component | License | Permitted Uses |
|---|---|---|
| **Source Code (Full Repository)** | [GNU AGPLv3](../LICENSE) | 100% Free: Usage, study, modification, distribution, and contributions under AGPLv3 copyleft terms. |
| **Documentation in `source/assets/`** | Project Authorship (Free Developer Adaptation) | Authored exclusively to meet Microsoft Store compliance; developers have full freedom to modify, delete, add, and adapt for their own forks and platforms. |
| **Third-Party Compilations (from Source)** | GNU AGPLv3 | Completely free to compile, run, package, and distribute under the developer's responsibility in full compliance with AGPLv3 terms. |
| **Author's Official Microsoft Store Binary** | Author's Official Release | Resale, third-party commercial redistribution, or reverse engineering of the author's official binaries is strictly prohibited. |
| **Third-Party Libraries (Qt 6, etc.)** | LGPL v3 / Compatible | Fully compliant with upstream open source attribution requirements. |

---

## 2. Open Source Rights (GNU AGPLv3)

The entire codebase of Munin App — including the low-level C++20 engine (`MuninEngine`), the benchmark utility (`MuninBench`), the standalone command-line interface (`munin-app-cli`), Qt 6 virtual models, custom widgets (`LogView`, `MinimapScrollBar`, `TimeSliceWidget`, `JsonInspectorWidget`), CMake presets, and mock tools — is distributed under the **GNU Affero General Public License v3 (AGPLv3)**:

```text
Munin App — Massive Log Viewer for Windows
Copyright (c) 2025-2026 Fernando Nillsson Cidade

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
```

---

## 3. Documentation in `source/assets/` & Complete Developer Freedom

The institutional and legal documentation maintained under `source/assets/` (`ABOUT`, `CLC`, `COPYRIGHT`, `EULA`, `LICENSES`, `NOTICES`, `PRIVACY_POLICY`, and `RELEASE`) is governed by the following guidelines:

1. **Microsoft Store Purpose:** Authored exclusively to fulfill compliance, legal, and operational mandates for the commercial release on the **Microsoft Store** (fulfilling EULA, ADA, privacy policy, and LGPD/GDPR marketplace governance mandates).
2. **Express Developer Authorization:** The author expressly authorizes all developers, contributors, and forks to make any changes, deletions, additions, or full rewrites of this documentation and source code. You have total freedom to adapt it as you see fit, provided you respect local regulations and platform publishing guidelines.
3. **Condition of Responsibility:** Developers are free to adapt or remove these texts in their own forks and distributions, provided they respect the guidelines, standards, and legislation in effect within their respective jurisdictions and target distribution platforms.

---

## 4. Distinction for Author's Official Binaries

The project operates under a pragmatic **Dual-Licensing** framework:
- **Open Source on GitHub:** The full source code is free and open under GNU AGPLv3. Any developer is free to audit, compile, generate custom builds, and distribute them under AGPLv3 terms.
- **Official Binary on Microsoft Store:** Distributed as a precompiled, digitally signed package for end users and organizations requiring frictionless installation and official store updates. Commercial EULA terms apply solely to the packaged binary distributed on the Microsoft Store.

</details>
