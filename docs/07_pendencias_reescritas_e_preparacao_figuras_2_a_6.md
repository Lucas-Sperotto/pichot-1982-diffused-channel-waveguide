# 07. Pendências reescritas e preparação das figuras 2 a 6

Este documento substitui uma lista genérica de pendências por um quadro operacional voltado à geração dos CSVs das figuras 2 a 6.

O foco aqui não é declarar que as figuras já foram reproduzidas, e sim deixar explícito:

- quais entradas já existem;
- quais saídas canônicas já podem ser geradas;
- o que ainda é apenas protótipo;
- o que ainda bloqueia validação científica.

## 1. Estado consolidado da infraestrutura

Neste estágio o repositório já dispõe de:

- casos estruturados em `data/input/figures/*.json`;
- um índice de preparação em `data/input/figures/manifest_figures_2_to_6.csv`;
- saídas organizadas em `out/figures/<figure_id>/<case_id>/`;
- um `output_manifest.json` por execução, além de `input_snapshot.json`, `run_summary.txt`, `profile_samples.csv` e do CSV canônico do estudo;
- geração em lote por `scripts/generate_figures_2_to_6_csvs.sh`.

Isso resolve a preparação de I/O para a campanha das figuras 2 a 6, mas não elimina as pendências numéricas do solver.

## 2. Pendências centrais que ainda afetam todas as figuras

- fechar a formulação vetorial completa do artigo em lugar do operador prototípico atual;
- refinar a busca modal além do critério atual de mínimo de `|det(A)|`;
- melhorar a regularização e a quadratura da contribuição de fronteira;
- medir convergência com refinamento de malha antes de declarar qualquer curva como validada;
- implementar reconstrução de campo para destravar a figura 5;
- confirmar, a partir do artigo, os parâmetros canônicos finais de cada figura e de cada curva de comparação.

## 3. Quadro por figura

### Figura 2

- entrada preparada: `data/input/figures/fig_02_homogeneous_rectangular_eq_integral.json`;
- saída canônica esperada: `dispersion_curve.csv`;
- diretório-alvo: `out/figures/fig_02/fig_02_homogeneous_rectangular_eq_integral/`;
- estado atual: executável com parâmetros provisórios;
- bloqueio principal: confirmar os parâmetros físicos históricos usados na comparação.

### Figura 3

- entrada preparada: `data/input/figures/fig_03_homogeneous_channel_eq_integral.json`;
- saída canônica esperada: `dispersion_curve.csv`;
- diretório-alvo: `out/figures/fig_03/fig_03_homogeneous_channel_eq_integral/`;
- estado atual: executável com geometria ainda sujeita a calibração;
- bloqueio principal: alinhar a geometria do guia com a configuração do artigo.

### Figura 4

- entradas preparadas:
  - `data/input/figures/fig_04_curve_A_diffused_1d_eq_integral.json`;
  - `data/input/figures/fig_04_curve_B_uniform_reference.json`;
- saídas canônicas esperadas: um `dispersion_curve.csv` por curva;
- diretórios-alvo:
  - `out/figures/fig_04/fig_04_curve_A_diffused_1d_eq_integral/`;
  - `out/figures/fig_04/fig_04_curve_B_uniform_reference/`;
- estado atual: é a família mais madura da coleção preparada;
- bloqueio principal: validar quantitativamente a curva protótipo contra a figura original.

### Figura 5

- entrada preparada: `data/input/figures/fig_05_field_map_preparation.json`;
- saída canônica desta etapa: `field_sampling_grid.csv`;
- diretório-alvo: `out/figures/fig_05/fig_05_field_map_preparation/`;
- estado atual: a malha de amostragem já é gerada com `status = pending_field_reconstruction`;
- bloqueio principal: reconstrução de campo para preencher `|E_y(x,y)|`.

### Figura 6

- entradas preparadas:
  - `data/input/figures/fig_06_curve_A_diffused_2d_eq_integral.json`;
  - `data/input/figures/fig_06_curve_B_uniform_reference.json`;
- saídas canônicas esperadas: um `dispersion_curve.csv` por curva;
- diretórios-alvo:
  - `out/figures/fig_06/fig_06_curve_A_diffused_2d_eq_integral/`;
  - `out/figures/fig_06/fig_06_curve_B_uniform_reference/`;
- estado atual: executável, porém ainda dependente de calibração do caso uniforme de referência;
- bloqueio principal: confirmar a parametrização média adotada na curva B.

## 4. Critério mínimo para considerar uma figura “preparada”

Uma figura passa a ser considerada preparada quando:

- existe pelo menos um caso JSON identificado por `figure_id`;
- o gerador em lote consegue produzir o diretório de saída previsto;
- o CSV canônico da figura é emitido com nome estável;
- o `output_manifest.json` deixa claro o status do artefato;
- o documento de pendências registra o que ainda é protótipo.

Isso ainda não equivale a “figura validada”.

## 5. Ordem recomendada de trabalho a partir daqui

1. manter a coleção das figuras 2 a 6 como fonte canônica de entradas reproduzíveis;
2. usar `scripts/generate_figures_2_to_6_csvs.sh` como verificação rápida da infraestrutura;
3. atacar primeiro convergência e calibração das figuras 2, 3, 4 e 6, que já produzem curvas;
4. tratar a figura 5 como caso preparado de I/O, mas ainda bloqueado por física ausente;
5. só iniciar comparação quantitativa figura a figura depois que a sensibilidade à malha estiver melhor controlada.
