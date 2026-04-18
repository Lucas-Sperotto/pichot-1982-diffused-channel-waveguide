# 06. Auditoria consolidada da Fase 1

Este documento passa a consolidar as funções que antes estavam distribuídas entre:

- `docs/06_auditoria_inicial_do_repositorio.md`;
- `docs/07_pendencias_reescritas_e_preparacao_figuras_2_a_6.md`;
- `docs/08_auditoria_repositorio_preparacao_figuras_2_a_6.md`.

Os arquivos `07` e `08` permanecem no repositório apenas como notas editoriais de redirecionamento. O conteúdo canônico de auditoria da Fase 1 fica mantido aqui.

## 1. Veredito executivo

O repositório pode ser considerado documental e operacionalmente fechado para a Fase 1.

Em termos práticos, isso significa que:

- o problema físico, a geometria, os domínios e os perfis de índice já estão separados da implementação;
- a notação do artigo já está ligada aos nomes operacionais do código e dos arquivos JSON;
- as Figuras 2 a 6 já possuem casos executáveis identificáveis em `data/input/figures/`;
- o fluxo de build, execução e geração de artefatos já está centralizado em `scripts/`;
- os testes já cobrem leitura de entrada, núcleo de Green, montagem, busca modal protótipo e geração dos principais CSVs.

O que permanece aberto a partir daqui já não é fundação documental. É refinamento científico e numérico do solver.

## 2. Base documental hoje consolidada

Os documentos que formam a base canônica desta etapa são:

- `docs/00_titulo_autoria_resumo.md`, para identificação do artigo e do escopo geral;
- `docs/01_introducao.md`, para o contexto histórico e bibliográfico;
- `docs/02_teoria.md`, para a tradução fiel da formulação do artigo;
- `docs/02_formulacao_do_problema_de_valor_de_contorno.md`, para a convenção operacional adotada no repositório;
- `docs/02_symbol_dictionary.md`, para a notação e a equivalência com nomes de código;
- `docs/03_resultados_numericos.md`, agora consolidando também o antigo recorte de exemplos e figuras;
- `docs/04_conclusoes.md` e `docs/05_referencias.md`, para o fechamento científico do texto traduzido;
- `docs/12_trilha_equacoes_para_codigo.md`, para a ponte entre equações e implementação.

Essa organização reduz duplicação e torna explícito onde termina a tradução do artigo e onde começa a leitura operacional do repositório.

## 3. Estrutura do repositório e fluxo operacional

Na data desta consolidação, a estrutura desejada pelo projeto já aparece de forma prática em:

- `src/`, para a implementação em C++;
- `tests/`, para testes unitários e de fumaça;
- `docs/`, para a base documental auditável;
- `data/`, para entradas reproduzíveis, manifests e referências auxiliares;
- `scripts/`, para build, execução e geração do lote principal de figuras;
- `out/`, para saídas por caso e por figura.

O fluxo principal ficou estável assim:

1. compilar com `scripts/build.sh`;
2. executar um caso com `scripts/run_case.sh <arquivo.json>`;
3. gerar o lote das figuras com `scripts/generate_figures_2_to_6_csvs.sh`.

Isso substitui a fase inicial em que havia divergência entre a estrutura desejada e o conteúdo realmente executável do repositório.

## 4. Auditoria consolidada das figuras 2 a 6

### Figura 2

- caso: `data/input/figures/fig_02_homogeneous_rectangular_eq_integral.json`;
- modo-alvo: $E^y_{11}$;
- perfil: homogêneo;
- artefato principal: `dispersion_curve.csv`;
- status nesta fase: caso calibrado em parâmetros, normalização de eixos e caminho de execução.

### Figura 3

- caso: `data/input/figures/fig_03_homogeneous_channel_eq_integral.json`;
- modo-alvo: $E^y_{11}$;
- perfil: homogêneo em meio estratificado assimétrico;
- artefato principal: `dispersion_curve.csv`;
- status nesta fase: caso calibrado em parâmetros, normalização de eixos e caminho de execução.

### Figura 4

- casos:
  - `data/input/figures/fig_04_curve_A_diffused_1d_eq_integral.json`;
  - `data/input/figures/fig_04_curve_B_uniform_reference.json`;
- modo-alvo: $E^x_{11}$;
- perfis: `parabolic_1d` para a curva A e homogêneo com índice médio para a curva B;
- artefato principal: `dispersion_curve.csv` em cada curva;
- status nesta fase: a distinção entre a curva difundida e a referência uniforme está documentada e operacionalizada.

### Figura 5

- caso: `data/input/figures/fig_05_field_map_preparation.json`;
- modo-alvo: $E^y_{21}$;
- perfil: `parabolic_1d`;
- artefatos principais e auxiliares:
  - `field_map.csv`;
  - `field_sampling_grid.csv`;
  - `mode_coefficients.csv`;
  - `field_map_status.txt`;
- observação de rastreabilidade: o caso hoje declara `field_sampling_grid.csv` como `canonical_csv_name`, mas o repositório preserva `field_map.csv` como artefato do campo reconstruído;
- status nesta fase: a figura deixou de ser apenas uma grade de amostragem e passou a produzir um mapa de campo auditável.

### Figura 6

- casos:
  - `data/input/figures/fig_06_curve_A_diffused_2d_eq_integral.json`;
  - `data/input/figures/fig_06_curve_B_uniform_reference.json`;
- modo-alvo: $E^x_{11}$;
- perfis: `circular_2d` para a curva A e homogêneo com índice médio para a curva B;
- artefato principal: `dispersion_curve.csv` em cada curva;
- status nesta fase: a família de casos já está calibrada e separada entre perfil difundido e referência uniforme.

## 5. Auditoria do solver protótipo

O repositório já saiu do estágio de infraestrutura vazia. Nesta consolidação, o solver protótipo já possui:

- Green escalar decomposta em `G^S + G^NS`;
- derivadas analíticas da parte singular e derivadas numéricas mais robustas da parte não singular;
- discretização de $\mathcal{D}_2$ por células retangulares com funções-base step;
- montagem separada entre identidade residual, termo escalar, termo regular de gradiente e termo distributivo de fronteira;
- busca modal guiada por `modal_residual`, mantendo `|det(A)|` como diagnóstico;
- reconstrução de coeficientes modais e mapa de campo para a Figura 5.

Ao mesmo tempo, as limitações permanecem explicitamente registradas:

- a formulação vetorial completa do artigo ainda não está fechada;
- a busca modal ainda não é um resolvedor rigoroso dos zeros exatos de $\det(A)$;
- a quadratura e a regularização da fronteira ainda são aproximações auditáveis;
- a nova quadratura oscilatória de `G_NS` aumentou o custo do lote completo das figuras, o que desloca parte do trabalho para a estabilização numérica da Fase 3.

## 6. Auditoria de saídas, scripts e testes

### Saídas

As execuções já produzem artefatos organizados por caso, incluindo:

- `results.csv`;
- `dispersion_curve.csv` ou `field_map.csv`, conforme o tipo de estudo;
- `profile_samples.csv`;
- `output_manifest.json`;
- `mode_coefficients.csv` e `field_map_status.txt` quando aplicável.

### Scripts

Os scripts principais já estão claros e reproduzíveis:

- `scripts/build.sh`;
- `scripts/run_case.sh`;
- `scripts/generate_figures_2_to_6_csvs.sh`.

### Testes

Os testes que sustentam esta etapa são:

- `tests/run_green_function_test.sh`;
- `tests/run_matrix_solver_test.sh`;
- `tests/smoke_case_loading.sh`;
- `tests/smoke_figures_csv_generation.sh`.

Eles já cobrem tanto o caminho operacional mínimo quanto blocos matemáticos sensíveis do protótipo.

## 7. Pendências que passam para a Fase 3

As pendências remanescentes já pertencem à fase de validação científica e refinamento do solver:

- fechar a formulação vetorial completa do artigo;
- substituir a busca modal aproximada por um método mais rigoroso para localizar zeros de $\det(A)$;
- reduzir o custo da quadratura oscilatória de `G_NS` sem perder consistência;
- medir convergência de malha para as figuras de dispersão;
- comparar quantitativamente as curvas com referências externas digitalizadas;
- confirmar a identificação modal da Figura 5 com critérios mais fortes do que a simples reconstrução do campo.

## 8. Fechamento desta auditoria

Depois desta consolidação, a leitura recomendada da Fase 1 fica assim:

- `docs/02_formulacao_do_problema_de_valor_de_contorno.md` para a formulação física mínima preservada no repositório;
- `docs/02_symbol_dictionary.md` para a notação;
- `docs/03_resultados_numericos.md` para os casos, figuras e papel dos testes;
- `docs/12_trilha_equacoes_para_codigo.md` para a abertura da Fase 2;
- `PLAN.md` e `TODO.md` para o que já foi concluído e o que segue em aberto.

Em outras palavras, o repositório já não precisa “descobrir o que reproduzir”. O próximo trabalho é tornar a reprodução numericamente mais fiel e cientificamente validada.

---
**Navegação:** [00 Resumo](00_titulo_autoria_resumo.md) | [01 Introdução](01_introducao.md) | [02 Formulação](02_formulacao_do_problema_de_valor_de_contorno.md) | [02 Símbolos](02_symbol_dictionary.md) | [02 Teoria](02_teoria.md) | [03 Resultados](03_resultados_numericos.md) | [04 Conclusões](04_conclusoes.md) | [05 Referências](05_referencias.md) | [06 Auditoria](06_auditoria_inicial_do_repositorio.md) | [12 Trilha do Código](12_trilha_equacoes_para_codigo.md) | [Plano](../PLAN.md) | [TODO](../TODO.md)
