# Plano de Projeto: Reprodução de Pichot (1982)

Este documento descreve o plano de trabalho para a reprodução do artigo de Pichot et al. (1982), dividido em fases que marcam o progresso desde a fundação teórica até a validação final e o empacotamento do projeto.

---

### Fase 1: Fundação Documental e Estrutural (✅ Concluída)

O objetivo desta fase foi estabelecer a base para um trabalho reproduzível.

- **Tradução do Problema:** O problema físico, a notação e as equações do artigo foram documentados em `docs/`.
- **Estrutura do Repositório:** A estrutura de diretórios foi definida para separar código (`src/`), dados de entrada (`data/`), scripts (`scripts/`), testes (`tests/`) e documentação (`docs/`).
- **Casos de Teste Reproduzíveis:** Os alvos de simulação (Figuras 2 a 6) foram mapeados para arquivos de entrada JSON em `data/input/figures/`, permitindo a execução rastreável de cada caso.

*Esta fase está documentada e auditada em `docs/06_auditoria_inicial_do_repositorio.md`.*

---

### Fase 2: Implementação do Solver Protótipo (✅ Concluída)

Nesta fase, um primeiro solver funcional foi desenvolvido, capaz de gerar os artefatos numéricos preliminares.

- **Discretização e Base:** Implementação do Método dos Momentos com funções-base do tipo "step" (constantes por célula) e testes por colocação, conforme descrito no artigo.
- **Núcleo de Green:** Implementação da função de Green para o meio estratificado, separando as contribuições singular e não singular.
- **Montagem da Matriz:** Desenvolvimento de um montador de matriz com tradução vetorial explícita dos termos escalar, regular e de fronteira, depois consolidada e auditada na Fase 3.
- **Busca Modal e Reconstrução:** Implementação de uma busca modal baseada na minimização do residual do operador e reconstrução de campos a partir dos coeficientes do vetor modal.

*A trilha detalhada das equações do artigo para o código do protótipo está em `docs/12_trilha_equacoes_para_codigo.md`.*

---

### Fase 3: Validação Científica e Refinamento do Solver (🎯 Em Andamento)

Esta é a fase atual do projeto. O foco é transformar os resultados do protótipo em uma reprodução cientificamente robusta e validada.

**3.0. Fase 3A: Primeiro marco fechado na Figura 2 (✅ Implementada)**
- A Eq. (3) e a Eq. (4) passaram a ser traduzidas operacionalmente por uma montagem vetorial explícita com funções-base `step` e teste por colocação, preservando a separação entre termo escalar volumétrico, termo regular com `grad' G` e termo distributivo de fronteira.
- A busca de `beta` deixou de ser guiada por `modal_residual` e passou a priorizar `|det(A)|`, mantendo o residual modal apenas como diagnóstico do vetor modal estimado.
- A Figura 2 passou a ter uma validação reproduzível inicial contra a curva "integral equation" digitizada do artigo, com artefatos automáticos de comparação, métricas e gráfico overlay.

**3.1. Estabilização Numérica do Protótipo (✅ marco 3.1A implementado)**
- [OK] Consolidar a nova quadratura oscilatória de `G_NS` e medir seu custo computacional no lote das Figuras 2 a 6.
  Resultado desta etapa: `scripts/benchmark_figures_2_to_6.sh` passou a executar o manifest científico completo, gravando `benchmark_cases.csv`, `benchmark_summary.json` e um `performance_summary.json` por caso em `out/benchmarks/...`.
- [OK] Reduzir o custo da avaliação de `G_NS`, `dG_NS/dx'` e `dG_NS/dy'` sem perder consistência com os testes de Green.
  Resultado desta etapa: `src/green_function.cpp` agora usa um avaliador interno em bloco para `G_NS`, `dG_NS/dx'` e `dG_NS/dy'`, compartilhando por amostra de quadratura os termos `gamma_1`, `gamma_3`, o coeficiente de reflexão, o envelope exponencial e a fase trigonométrica; `src/matrix_solver.cpp` passou a reutilizar esse bundle tanto no volume quanto na integração de fronteira.
- [OK] Revisar o tratamento singular remanescente de `G^S` para aproximar a média de célula do comportamento integral esperado pelo artigo.
  Resultado desta etapa: a auto-interação de `G^S` deixou de usar apenas a média regularizada por subcélulas e passou a separar explicitamente a parte singular por uma quadratura simétrica por quadrantes com mapeamento tipo Duffy/log-aware, mantendo `G_NS` como parte regular integrada numericamente na célula.

**3.2. Finalização da Formulação Vetorial**
- [OK] Fechar a tradução vetorial explícita do termo `grad' G` da equação integral (3) na discretização atual com base `step` e colocação.
- [OK] Revisar os blocos `A_xy` e `A_yx` para que o acoplamento vetorial fique consistente com a formulação documentada em `docs/02_teoria.md` e `docs/12_trilha_equacoes_para_codigo.md`.
- [OK] Usar o caso homogêneo (Fig. 2) como primeiro teste de regressão e validação operacional da formulação vetorial refinada.

**3.3. Validação Quantitativa das Curvas de Dispersão**
- [OK] Digitalizar a curva "integral equation" da Figura 2 e armazená-la em `data/reference/`.
- [OK] Gerar `comparison.csv`, `metrics.json` e `overlay.png` para a Figura 2 no fluxo padrão de execução.
- [ ] Digitalizar as curvas de referência (Goell, Yeh, Marcatili) das imagens do artigo.
- [ ] Armazenar os dados de referência em arquivos CSV (`data/reference/`).
- [ ] Desenvolver scripts para gerar gráficos sobrepostos e calcular métricas de erro (ex: Erro Quadrático Médio).
- [ ] Realizar estudos de convergência de malha (`Nx`, `Ny`) para garantir a precisão dos resultados.

**3.4. Verificação da Identificação Modal (Figura 5)**
- [ ] Inspecionar visualmente o mapa de campo reconstruído para confirmar a estrutura do modo $E^y_{21}$ (dois máximos em `x`, um em `y`).
- [ ] Analisar a simetria do campo para garantir que corresponde à esperada para o modo.
- [ ] Transformar essa inspeção em checagens reprodutíveis sempre que possível, evitando depender apenas de avaliação visual.

**3.5. Aprimoramento da Busca Modal**
- [OK] Substituir a busca atual guiada por `modal_residual` por um fluxo cujo critério principal seja `det(A) = 0`.
- [OK] Garantir que a busca modal refinada preserve rastreabilidade entre `beta`, `det(A)`, residual modal e vetor modal estimado.

---

### Fase 4: Empacotamento e Documentação Final (⏳ Próxima Fase)

Após a validação científica, o projeto será preparado para distribuição e arquivamento.

- [ ] Revisar e finalizar o `README.md` com instruções detalhadas de compilação, execução e reprodução das figuras.
- [ ] Consolidar um fluxo único de reprodução completa das figuras e artefatos finais via `scripts/`.
- [ ] Gerar as versões finais de todas as figuras e artefatos de saída.
- [ ] Escrever uma discussão final sobre os resultados, a precisão alcançada e as limitações da implementação.
- [ ] Revisar os documentos de auditoria e trilha técnica para refletir o estado final do solver.
- [ ] Limpar o código e garantir que o repositório esteja autocontido e bem organizado.
