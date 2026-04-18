# TODO

Este arquivo consolida as pendências abertas ao final da Fase 2 e o ponto de partida da Fase 3.

O objetivo é manter, na raiz do repositório, uma visão curta do que ainda precisa ser acompanhado sem espalhar a leitura por vários documentos de auditoria.

## 1. Leitura executiva da Fase 3A

- [OK] Fechar a formulação vetorial explícita da Eq. (3) e da Eq. (4) na discretização atual com funções-base `step` e colocação.
  Resultado desta etapa: `src/matrix_solver.cpp` agora monta explicitamente os blocos vetoriais a partir da componente observada de `grad' G` e da componente-fonte de $\varepsilon \,\mathrm{grad}(1/\varepsilon)$, preservando a separação entre termo escalar, termo regular e termo de fronteira.
- [OK] Trocar a busca modal para priorizar zeros de $\det(A)$.
  Resultado desta etapa: `find_beta_root` passou a fazer varredura e refinamento local guiados por `|det(A)|`, enquanto `modal_residual` permanece apenas como diagnóstico secundário salvo nas saídas.
- [OK] Gerar uma validação reproduzível inicial da Figura 2 contra a curva "integral equation" digitizada do artigo.
  Resultado desta etapa: `scripts/run_case.sh` passou a chamar `scripts/postprocess_case.py`, que gera `fig_02_integral_equation_comparison.csv`, `fig_02_integral_equation_metrics.json` e `fig_02_integral_equation_overlay.png` a partir de `data/reference/fig_02_integral_equation_digitized.csv`.

## 2. Pendências científico-numéricas para a Fase 2 em diante

- [OK] Abrir uma trilha documental explícita entre as equações do artigo e os passos do código atual.
  Resultado desta revisão: `docs/12_trilha_equacoes_para_codigo.md` agora descreve a discretização de $\mathcal{D}_2$, as funções-base step, a montagem de `A`, a avaliação de `G = G_S + G_NS` e o critério operacional hoje usado para localizar $\beta$.
- [ ] Melhorar a regularização e a quadratura da contribuição de fronteira.
- [ ] Medir convergência de malha para as figuras 2, 3, 4 e 6 antes de declarar as curvas como reproduções finais.
- [ ] Comparar quantitativamente as curvas geradas com as demais referências externas digitalizadas ou tabuladas.
- [ ] Verificar se o campo reconstruído da Figura 5 representa fielmente o modo $E^y_{21}$, e não apenas um vetor quase-nulo no $\beta$ informado pelo artigo.

## 3. Pendências operacionais de manutenção

- [ ] Manter o `README.md` e os documentos de execução alinhados com o fluxo real baseado em `scripts/build.sh`, `scripts/run_case.sh` e `scripts/generate_figures_2_to_6_csvs.sh`.
- [ ] Reavaliar periodicamente os testes de fumaça quando o contrato de saída (`results.csv`, `dispersion_curve.csv`, `field_map.csv`, `output_manifest.json`) mudar.

## 4. Leitura rápida do status da Fase 2


A abertura formal da Fase 2 agora está em `docs/12_trilha_equacoes_para_codigo.md`.
