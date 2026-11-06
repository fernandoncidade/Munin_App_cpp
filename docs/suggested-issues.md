# Sugestões de Issues Iniciais / Starter Issues — Munin App

<p align="center">
  <b>Selecione o idioma / Select language:</b><br>
  <a href="#ptbr">🇧🇷 Português (BR)</a> | <a href="#enus">🇺🇸 English (US)</a>
</p>

---

## <a id="ptbr"></a>🇧🇷 Português (BR)

<details>
<summary>Clique para expandir / recolher as sugestões de issues em português</summary><br>

# Issues Recomendadas para Engajamento da Comunidade

Abaixo estão templates de issues estruturadas para publicação em [Issues -> New Issue](../../issues/new), facilitando a colaboração de desenvolvedores em C++ e análise de logs:

---

### Issue 1: Aceleração Vetorial com Instruções AVX-512
- **Título:** `[PERF] Support AVX-512 vector acceleration for 64-byte newline scanning`
- **Labels:** `enhancement`, `performance`, `simd`
- **Conteúdo:**
```markdown
### Resumo
Atualmente, o `SimdUtils` utiliza instruções AVX2 (256 bits, 32 bytes por ciclo) e SSE4.2 (128 bits, 16 bytes por ciclo).

### Melhoria Proposta
Adicionar caminho de execução com instruções AVX-512 (`_mm512_cmpeq_epi8_mask`) para escanear 64 bytes por ciclo de clock em processadores compatíveis (Intel Xeon, Core 11th+ e AMD Zen 4+).

### Critérios de Aceite
- [ ] Detecção em tempo de execução via `__cpuid` para suporte a AVX-512F e AVX-512BW.
- [ ] Rotina de escaneamento em `SimdUtils.cpp` atingindo taxas superiores a 5 GB/s em memória.
- [ ] Fallback transparente para AVX2 e SSE4.2 mantido.
```

---

### Issue 2: Exportação de Recorte Filtrado de Logs
- **Título:** `[FEATURE] Export filtered log lines to new standalone file`
- **Labels:** `enhancement`, `ui`
- **Conteúdo:**
```markdown
### Resumo
Usuários frequentemente aplicam filtros por Regex ou Time-Slice para isolar uma janela de erro de 10 minutos ou incidentes específicos.

### Melhoria Proposta
Adicionar ação "Exportar Linhas Filtradas" (menu Arquivo e botão na barra de busca) para salvar o recorte selecionado diretamente em disco sem necessidade de copiar e colar.

### Critérios de Aceite
- [ ] Opção de salvar em `.log`, `.txt` ou `.json`.
- [ ] Processamento de escrita em streaming para evitar alocações de memória.
- [ ] Barra de progresso para recortes com mais de 100.000 linhas.
```

---

### Issue 3: Marcadores Visuais e Favoritos de Linha (Bookmarking)
- **Título:** `[FEATURE] Line bookmarking and quick jump navigation`
- **Labels:** `enhancement`, `good first issue`
- **Conteúdo:**
```markdown
### Resumo
Durante a depuração de incidentes complexos, analistas precisam marcar linhas de interesse para retornar a elas rapidamente.

### Melhoria Proposta
Permitir que o usuário clique na margem esquerda da tabela ou pressione `Ctrl+F2` para marcar linhas. Exibir indicadores no Minimapa de densidade.
```

</details>

---

## <a id="enus"></a>🇺🇸 English (US)

<details>
<summary>Click to expand / collapse starter issues in English</summary><br>

# Recommended Issues for Community Engagement

Below are structured issue templates ready for publication under [Issues -> New Issue](../../issues/new):

---

### Issue 1: AVX-512 Vector Acceleration
- **Title:** `[PERF] Support AVX-512 vector acceleration for 64-byte newline scanning`
- **Labels:** `enhancement`, `performance`, `simd`
- **Content:**
```markdown
### Summary
`SimdUtils` currently utilizes AVX2 (256-bit, 32 bytes/cycle) and SSE4.2 (128-bit, 16 bytes/cycle).

### Proposed Enhancement
Add an AVX-512 execution path (`_mm512_cmpeq_epi8_mask`) to process 64 bytes per clock cycle on compatible CPUs (Intel Xeon, Core 11th+ and AMD Zen 4+).

### Acceptance Criteria
- [ ] Runtime `__cpuid` detection for AVX-512F and AVX-512BW.
- [ ] Scanning implementation in `SimdUtils.cpp` exceeding 5 GB/s throughput.
- [ ] Transparent fallback to AVX2 and SSE4.2 preserved.
```

---

### Issue 2: Export Filtered Log Slices
- **Title:** `[FEATURE] Export filtered log lines to new standalone file`
- **Labels:** `enhancement`, `ui`
- **Content:**
```markdown
### Summary
Users frequently apply Regex or Time-Slice filters to isolate an incident window or specific errors.

### Proposed Enhancement
Add an "Export Filtered Lines" action in File menu and search toolbar to save the current slice directly to disk without copying and pasting.

### Acceptance Criteria
- [ ] Option to save as `.log`, `.txt`, or `.json`.
- [ ] Streaming write processing to prevent heap memory allocations.
- [ ] Progress bar indicator for slices larger than 100,000 lines.
```

---

### Issue 3: Line Bookmarking & Navigation
- **Title:** `[FEATURE] Line bookmarking and quick jump navigation`
- **Labels:** `enhancement`, `good first issue`
- **Content:**
```markdown
### Summary
Analysts investigating incidents need to bookmark key lines and return to them quickly.

### Proposed Enhancement
Allow users to click the left gutter or press `Ctrl+F2` to toggle bookmarks, displaying color markers on the Minimap scrollbar.
```

</details>
