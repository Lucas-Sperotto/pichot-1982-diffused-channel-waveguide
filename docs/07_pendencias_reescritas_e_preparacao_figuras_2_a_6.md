# 07. Pendências reescritas para as figuras 2 a 6

Este documento deixa de tratar as figuras 2 a 6 apenas como “preparação de I/O”.

Nesta etapa, o repositório já passou a:

- usar parâmetros geométricos e materiais lidos das próprias figuras do artigo;
- gerar curvas de dispersão com a abcissa normalizada do artigo;
- registrar, além de `|det(A)|`, um residual modal baseado no vetor quase-nulo do sistema discretizado;
- reconstruir um mapa de campo para a figura 5 a partir de um vetor modal estimado.

Isso melhora substancialmente a rastreabilidade, mas ainda não equivale à validação científica final.

Após a revisão documental da Fase 1, a definição de quais figuras entram no escopo, qual arquivo representa cada caso e como cada caso será resolvido deixou de estar dispersa: essa fundação agora está consolidada em `docs/03.3_exemplos.md` e `docs/03_resultados_numericos.md`.

## 1. O que deixou de ser pendência

- a calibração básica dos casos das figuras 2, 3, 4 e 6 deixou de ser provisória: os parâmetros agora refletem os valores que aparecem nas imagens do artigo;
- a figura 5 deixou de gerar apenas uma grade vazia e passou a produzir `field_map.csv`, `field_sampling_grid.csv`, `mode_coefficients.csv` e `field_map_status.txt`;
- a saída das curvas deixou de depender de uma abcissa interna inconsistente e passou a usar a variável normalizada mostrada nas figuras, isto é,
  $$
  \frac{2b}{\lambda_0}\sqrt{n_\text{alto}^2-n_\text{baixo}^2};
  $$
- a execução agora registra o residual modal estimado a partir do vetor quase-nulo de $A$.

## 2. Pendências centrais que continuam em aberto

As pendências desta seção já não são lacunas de identificação de caso ou de organização documental. Elas pertencem ao fechamento científico e numérico das fases seguintes.

- fechar a formulação vetorial completa do artigo no lugar do operador escalar/vetorial híbrido atual;
- substituir a busca modal aproximada por um procedimento mais rigoroso de localização dos zeros de $\det(A)$;
- melhorar a regularização e a quadratura da contribuição de fronteira;
- medir convergência de malha antes de declarar as curvas como reproduções finais;
- comparar quantitativamente as curvas geradas com referências externas digitizadas ou tabuladas;
- verificar se a estrutura modal estimada para a figura 5 representa fielmente o modo $E^y_{21}$, e não apenas um vetor quase-nulo no beta informado pelo artigo.

## 3. Estado por figura

### Figura 2

- entrada: `data/input/figures/fig_02_homogeneous_rectangular_eq_integral.json`;
- parâmetros calibrados na própria figura: `n1 = n3 = 1.01`, `n2 = 1.05`, `a = 2b`;
- saída canônica: `dispersion_curve.csv`;
- novo status: caso calibrado em parâmetros e escala;
- pendência remanescente: comparação quantitativa com Goell, FEM, Marcatili e effective index.

### Figura 3

- entrada: `data/input/figures/fig_03_homogeneous_channel_eq_integral.json`;
- parâmetros calibrados na própria figura: `n1 = 1.0`, `n2 = 1.5`, `n3 = 1.43`, `a = b`;
- saída canônica: `dispersion_curve.csv`;
- novo status: caso calibrado em parâmetros e escala;
- pendência remanescente: validar quantitativamente a curva contra Yeh, Marcatili e effective index.

### Figura 4

- entradas:
  - `data/input/figures/fig_04_curve_A_diffused_1d_eq_integral.json`;
  - `data/input/figures/fig_04_curve_B_uniform_reference.json`;
- parâmetros calibrados na própria figura:
  - curva A: `n1 = 1.0`, `n3 = 1.44`, `n2m = 1.50`, `a = 2b`;
  - curva B: `n1 = 1.0`, `n3 = 1.44`, `\bar{n}_2 = 1.48`, `a = 2b`;
- saída canônica: `dispersion_curve.csv` para cada curva;
- novo status: família calibrada em parâmetros e escala;
- pendência remanescente: confronto quantitativo com a curva de Yeh e com o método do índice efetivo modificado.

### Figura 5

- entrada: `data/input/figures/fig_05_field_map_preparation.json`;
- parâmetros calibrados na própria figura: `lambda0 = 0.6328 um`, `beta/k0 = 1.4447`, `a = 2.22 um`, `a = 2b`, `n1 = 1.0`, `n3 = 1.44`, `n2m = 1.5`;
- saídas canônicas e auxiliares:
  - `field_map.csv`;
  - `field_sampling_grid.csv`;
  - `mode_coefficients.csv`;
  - `field_map_status.txt`;
- novo status: a figura deixou de ser apenas um grid de amostragem;
- pendência remanescente: confirmar se o campo reconstruído representa fielmente o modo $E^y_{21}$ da formulação completa do artigo.

### Figura 6

- entradas:
  - `data/input/figures/fig_06_curve_A_diffused_2d_eq_integral.json`;
  - `data/input/figures/fig_06_curve_B_uniform_reference.json`;
- parâmetros calibrados na própria figura:
  - curva A: `n1 = 1.0`, `n3 = 1.44`, `n2m = 1.50`, `a = 2b`;
  - curva B: `n1 = 1.0`, `n3 = 1.44`, `\bar{n}_2 = 1.47`, `a = 2b`;
- saída canônica: `dispersion_curve.csv` para cada curva;
- novo status: casos calibrados em parâmetros e escala;
- pendência remanescente: validação quantitativa da separação entre as curvas A e B e teste de convergência.

## 4. Critério mínimo atualizado para considerar uma figura “resolvida nesta etapa”

Uma figura é considerada resolvida nesta etapa quando:

- usa os parâmetros visíveis na própria figura do artigo;
- gera um CSV canônico com a mesma normalização de eixos mostrada no artigo;
- deixa explícito, via `output_manifest.json` e `run_summary.txt`, o status numérico do artefato;
- no caso da figura 5, produz valores de campo e não apenas uma grade de amostragem.

Isso ainda é um marco de reprodutibilidade operacional, não a palavra final sobre fidelidade científica.

## 5. Próxima ordem de ataque

1. medir convergência de malha para as figuras 2, 3, 4 e 6 com a nova abcissa calibrada;
2. inspecionar a seleção modal para reduzir o risco de capturar o modo errado perto do corte;
3. comparar quantitativamente as curvas com referências externas digitizadas;
4. revisar a figura 5 com critérios modais mais fortes para o modo $E^y_{21}$.

## 6. Leitura para fechamento da Fase 1

Do ponto de vista documental, as figuras 2 a 6 já podem ser consideradas fechadas para a Fase 1 quando observadas em conjunto com:

- `data/input/figures/manifest_figures_2_to_6.csv`;
- os arquivos JSON em `data/input/figures/`;
- `docs/03.3_exemplos.md`;
- `docs/03_resultados_numericos.md`.

Isso significa que:

- cada figura já possui um caso executável identificável;
- cada caso já possui modo-alvo, perfil material e saída canônica declarados;
- a diferença entre curvas principais, curvas de referência e mapa de campo já está documentada;
- as pendências que restam nesta nota já não bloqueiam o fechamento documental da Fase 1.
