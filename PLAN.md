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
- **Montagem da Matriz:** Desenvolvimento de um montador de matriz que inclui os termos escalar, de gradiente regular e de fronteira, ainda que de forma aproximada (operador híbrido).
- **Busca Modal e Reconstrução:** Implementação de uma busca modal baseada na minimização do residual do operador e reconstrução de campos a partir dos coeficientes do vetor modal.

*A trilha detalhada das equações do artigo para o código do protótipo está em `docs/12_trilha_equacoes_para_codigo.md`.*

---

### Fase 3: Validação Científica e Refinamento do Solver (🎯 Em Andamento)

Esta é a fase atual do projeto. O foco é transformar os resultados do protótipo em uma reprodução cientificamente robusta e validada.

**3.1. Estabilização Numérica do Protótipo**
- [ ] Consolidar a nova quadratura oscilatória de `G_NS` e medir seu custo computacional no lote das Figuras 2 a 6.
- [ ] Reduzir o custo da avaliação de `G_NS`, `dG_NS/dx'` e `dG_NS/dy'` sem perder consistência com os testes de Green.
- [ ] Revisar o tratamento singular remanescente de `G^S` para aproximar a média de célula do comportamento integral esperado pelo artigo.

**3.2. Finalização da Formulação Vetorial**
- [ ] Fechar a tradução vetorial completa do termo `grad' G` da equação integral (3), sem depender do operador híbrido atual.
- [ ] Revisar e completar os blocos `A_xy` e `A_yx` para que o acoplamento vetorial fique consistente com a formulação documentada em `docs/02_teoria.md` e `docs/12_trilha_equacoes_para_codigo.md`.
- [ ] Usar o caso homogêneo (Fig. 2) como primeiro teste de regressão e validação da formulação vetorial refinada.

**3.3. Validação Quantitativa das Curvas de Dispersão**
- [ ] Digitalizar as curvas de referência (Goell, Yeh, Marcatili) das imagens do artigo.
- [ ] Armazenar os dados de referência em arquivos CSV (`data/reference/`).
- [ ] Desenvolver scripts para gerar gráficos sobrepostos e calcular métricas de erro (ex: Erro Quadrático Médio).
- [ ] Realizar estudos de convergência de malha (`Nx`, `Ny`) para garantir a precisão dos resultados.

**3.4. Verificação da Identificação Modal (Figura 5)**
- [ ] Inspecionar visualmente o mapa de campo reconstruído para confirmar a estrutura do modo $E^y_{21}$ (dois máximos em `x`, um em `y`).
- [ ] Analisar a simetria do campo para garantir que corresponde à esperada para o modo.
- [ ] Transformar essa inspeção em checagens reprodutíveis sempre que possível, evitando depender apenas de avaliação visual.

**3.5. Aprimoramento da Busca Modal**
- [ ] Substituir a busca atual guiada por `modal_residual` por um método mais rigoroso para localizar os zeros de `det(A) = 0`.
- [ ] Garantir que a busca modal refinada preserve rastreabilidade entre `beta`, `det(A)`, residual modal e vetor modal estimado.

---

### Fase 4: Empacotamento e Documentação Final (⏳ Próxima Fase)

Após a validação científica, o projeto será preparado para distribuição e arquivamento.

- [ ] Revisar e finalizar o `README.md` com instruções detalhadas de compilação, execução e reprodução das figuras.
- [ ] Consolidar um fluxo único de reprodução completa das figuras e artefatos finais via `scripts/`.
- [ ] Gerar as versões finais de todas as figuras e artefatos de saída.
- [ ] Escrever uma discussão final sobre os resultados, a precisão alcançada e as limitações da implementação.
- [ ] Revisar os documentos de auditoria e trilha técnica para refletir o estado final do solver.
- [ ] Limpar o código e garantir que o repositório esteja autocontido e bem organizado.
