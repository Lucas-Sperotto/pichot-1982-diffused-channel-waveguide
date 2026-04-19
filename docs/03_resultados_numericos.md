# 3. Resultados numéricos

Nota editorial:
este arquivo passa a ser o ponto canônico para:

- o recorte das figuras-alvo;
- a descrição científica dos casos numéricos;
- a leitura operacional das Figuras 2 a 6;
- o papel dos casos auxiliares e dos testes automatizados.

As notas `docs/03.3_exemplos.md` e `docs/09_figuras.md` ficam preservadas apenas como apontadores editoriais para esta versão consolidada.

Vários exemplos são propostos aqui.

## 3.0. Escopo consolidado das figuras-alvo

### 3.0.1. Regra de escopo

A Figura 1 entra como referência geométrica e documental.

Os alvos numéricos de reprodução começam nas Figuras 2 a 6, porque são elas que definem os primeiros compromissos verificáveis do projeto:

- curvas de dispersão;
- comparação entre perfis homogêneos e difundidos;
- mapa de campo modal.

### 3.0.2. Tabela consolidada das figuras-alvo

| Figura | Papel no projeto | Modo-alvo | Perfil principal | Caso(s) do repositório | Artefato principal |
| --- | --- | --- | --- | --- | --- |
| Fig. 1 | geometria e notação; não é alvo numérico isolado | não se aplica | geometria conceitual do guia difundido | não se aplica | não se aplica |
| Fig. 2 | curva de dispersão para fibra retangular homogênea | $E^y_{11}$ | homogêneo | `data/input/figures/fig_02_homogeneous_rectangular_eq_integral.json` | `dispersion_curve.csv` |
| Fig. 3 | curva de dispersão para guia de canal homogêneo | $E^y_{11}$ | homogêneo | `data/input/figures/fig_03_homogeneous_channel_eq_integral.json` | `dispersion_curve.csv` |
| Fig. 4A | curva de dispersão do guia difundido parabólico 1-D | $E^x_{11}$ | `parabolic_1d` | `data/input/figures/fig_04_curve_A_diffused_1d_eq_integral.json` | `dispersion_curve.csv` |
| Fig. 4B | referência uniforme por índice médio para a Figura 4 | $E^x_{11}$ | homogêneo com $\bar{n}_2=1.48$ | `data/input/figures/fig_04_curve_B_uniform_reference.json` | `dispersion_curve.csv` |
| Fig. 5 | mapa de campo modal no guia difundido parabólico 1-D | $E^y_{21}$ | `parabolic_1d` | `data/input/figures/fig_05_field_map_preparation.json` | `field_map.csv` |
| Fig. 6A | curva de dispersão do guia difundido circular 2-D | $E^x_{11}$ | `circular_2d` | `data/input/figures/fig_06_curve_A_diffused_2d_eq_integral.json` | `dispersion_curve.csv` |
| Fig. 6B | referência uniforme por índice médio para a Figura 6 | $E^x_{11}$ | homogêneo com $\bar{n}_2=1.47$ | `data/input/figures/fig_06_curve_B_uniform_reference.json` | `dispersion_curve.csv` |

### 3.0.3. O que cada figura precisa preservar

#### Figura 2

- comparação da equação integral vetorial com referências clássicas para um núcleo homogêneo retangular;
- modo dominante $E^y_{11}$;
- mesma abcissa normalizada usada no artigo.

#### Figura 3

- caso homogêneo com superstrato e substrato diferentes;
- modo $E^y_{11}$;
- comparação com Yeh, Marcatili e método do índice efetivo.

#### Figura 4

- comparação entre um perfil difundido parabólico 1-D e uma referência uniforme de índice médio;
- modo $E^x_{11}$;
- necessidade de manter separadas as curvas A e B.

#### Figura 5

- reconstrução do campo $|E_y(x,y)|$;
- modo $E^y_{21}$;
- parâmetros do artigo explicitamente fixados no arquivo de entrada, incluindo $\lambda_0=0.6328\,\mu\text{m}$ e $\beta/k_0=1.4447$.

#### Figura 6

- comparação entre um perfil difundido circular 2-D e uma referência uniforme de índice médio;
- modo $E^x_{11}$;
- necessidade de preservar a distinção entre a curva A difundida e a curva B uniforme.

## 3.1. Fibra retangular homogênea

Yeh [9,10], utilizando o método dos elementos finitos, Goell [3], utilizando a técnica de expansão harmônica, e Marcatili [4], utilizando técnicas aproximadas, obtiveram as características de dispersão de guias de onda dielétricos retangulares.

Comparamos esses resultados com aqueles obtidos pelo método da equação integral vetorial. Os resultados são mostrados na Fig. 2 para o modo \(E^{y}_{11}\). Foi encontrado excelente acordo com os resultados de Goell, e observa-se uma diferença entre nossos resultados, os de Yeh e os de Marcatili nas proximidades da frequência de corte.

![Figura 2 — Comparação dos resultados da equação integral vetorial para o modo $E^{y}_{11}$ com o método dos elementos finitos, os resultados de Goell, o método aproximado de Marcatili e o método do índice efetivo.](img/fig_02.png)

Figura 2 — Comparação dos resultados da equação integral vetorial para o modo $E^{y}_{11}$ com o método dos elementos finitos, os resultados de Goell, o método aproximado de Marcatili e o método do índice efetivo.

### Leitura operacional da Figura 2

- arquivo operacional: `data/input/figures/fig_02_homogeneous_rectangular_eq_integral.json`;
- modo-alvo: $E^y_{11}$;
- perfil: homogêneo;
- parâmetros documentados na figura e no caso operacional: $n_1 = n_3 = 1.01$, $n_2 = 1.05$, $a = 2b$;
- curvas mostradas: integral equation and Goell, finite element, effective index e Marcatili.

As expressões documentais dos eixos da figura são:

$$
X = \left(\frac{2b}{\lambda_0}\right)\left(n_2^2 - n_1^2\right)^{1/2},
$$

$$
Y = \frac{\left(\frac{\beta}{k_0}\right)^2 - n_1^2}{n_2^2 - n_1^2}.
$$

As marcações visuais observadas na figura são compatíveis com uma malha horizontal de $0$ a $4.0$ em passos de $0.2$ e com uma malha vertical de $0$ a $1.0$ em passos de $0.2$.

Este caso será resolvido como uma curva de dispersão: o solver percorre a abcissa normalizada do artigo, converte cada valor em $\lambda_0$, monta o operador discretizado para `profile_type = "homogeneous"` e registra, para cada ponto, $\beta/k_0$, a ordenada normalizada, `det_abs` e `modal_residual`. A comparação quantitativa com Goell, FEM, effective index e Marcatili pertence à etapa de validação posterior.

Nota editorial da Fase 3A:

- a validação reproduzível inicial da Figura 2 passa a usar somente a curva "integral equation" do próprio artigo como referência digitizada;
- essa referência fica registrada em `data/reference/fig_02_integral_equation_digitized.csv`;
- o fluxo padrão de `scripts/run_case.sh` agora gera também:
  - `fig_02_integral_equation_comparison.csv`;
  - `fig_02_integral_equation_metrics.json`;
  - `fig_02_integral_equation_overlay.png`.

Nota editorial da Fase 3.1A:

- cada execução do solver passa a salvar também `performance_summary.json`, com tempo de parede do solver e contadores de uso de `G_NS`, de seus ramos numéricos e das auto-interações regularizadas;
- o benchmark reproduzível do lote das Figuras 2 a 6 passa a ser feito por `scripts/benchmark_figures_2_to_6.sh`.

Nota editorial da Fase 3.3A:

- a validação quantitativa da Figura 2 passa a ser guiada por `data/reference/fig_02_references_manifest.json`, em vez de depender de uma única referência hard-coded;
- a Figura 2 agora compara explicitamente as curvas `integral_equation`, `goell`, `yeh_fem`, `effective_index`, `marcatili_digitized` e `marcatili_external`;
- Marcatili passa a ter dupla rastreabilidade: uma curva digitizada do artigo e uma curva importada como CSV externo;
- o caso da Figura 2 passa a gerar também `validation_manifest.json` e `fig_02_validation_overlay.png`;
- a convergência de malha reproduzível desta etapa vale apenas para a Figura 2, com baseline interno na malha mais fina.

## 3.2. Guia de canal homogêneo

Mostra-se na Fig. 3 a comparação entre nossos resultados e aqueles obtidos por Yeh, Marcatili e pelo método do índice efetivo para o guia de canal homogêneo no modo $E^{y}_{11}$. Observa-se boa concordância entre a equação integral vetorial e os resultados de Yeh.

Em baixas frequências, a curva de Marcatili desvia dos resultados de Yeh e dos nossos.

![Figura 3 — Relação de dispersão de um guia de canal homogêneo. Comparação da equação integral vetorial com os resultados do método dos elementos finitos, a abordagem de Marcatili e o método do índice efetivo.](img/fig_03.png)

Figura 3 — Relação de dispersão de um guia de canal homogêneo. Comparação da equação integral vetorial com os resultados do método dos elementos finitos, a abordagem de Marcatili e o método do índice efetivo.

### Leitura operacional da Figura 3

- arquivo operacional: `data/input/figures/fig_03_homogeneous_channel_eq_integral.json`;
- modo-alvo: $E^y_{11}$;
- perfil: homogêneo;
- parâmetros documentados na figura e no caso operacional: $n_1 = 1.0$, $n_2 = 1.5$, $n_3 = 1.43$, $a = b$;
- curvas mostradas: integral equation and finite element, effective index e Marcatili.

As expressões documentais dos eixos da figura são:

$$
X = \left(\frac{2b}{\lambda_0}\right)\left(n_2^2 - n_3^2\right)^{1/2},
$$

$$
Y = \frac{\left(\frac{\beta}{k_0}\right)^2 - n_3^2}{n_2^2 - n_3^2}.
$$

As marcações visuais observadas na figura são compatíveis com uma malha horizontal de $0$ a $4.0$ em passos de $0.2$ e com uma malha vertical de $0$ a $1.0$ em passos de $0.2$.

Este caso será resolvido com o mesmo fluxo de curva de dispersão da Figura 2, mas em um meio estratificado assimétrico. O papel desta figura na Fase 1 é fixar um caso homogêneo mais próximo do guia de canal do artigo, preservando os eixos publicados e a lista de curvas de comparação.

## 3.3. Guias de canal inomogêneos

### 3.3.1. Difusão parabólica: modo $E^{x}_{11}$

Apresenta-se aqui a curva de dispersão para o modo $E^{x}_{11}$ no caso de um guia de canal difundido com difusão unidirecional ao longo do eixo $y$, com forma parabólica do índice de refração

$$
n_2(x,y)=n_3+\frac{(n_{2m}-n_3)(b^2-y^2)}{b^2}, \qquad 0<y<b
$$

onde $b$ é a profundidade de difusão, $n_{2m}$ é o índice de refração máximo na superfície e $n_3$ é o índice de refração do substrato.

Essa variação de índice foi investigada por Yeh [10]. Assume-se que o processo de difusão está limitado à região retangular do núcleo. A razão entre a abertura da máscara $a$ e a profundidade de difusão $b$ é igual a 2.

Na Fig. 4, comparamos o guia de canal difundido (curva A) com um guia de canal homogêneo que possui, no interior do núcleo, o índice médio $\bar{n}_2$ do guia difundido $(\bar{n}_2=1.48)$ (curva B). Também realizamos a comparação com o método do índice efetivo “modificado” e com o método de Yeh.

A concordância entre os resultados de Yeh e os da equação integral vetorial é boa. O método do índice efetivo “modificado” fornece uma boa aproximação dos resultados.

![Figura 4 — Curvas de dispersão para o modo dominante $E^{x}_{11}$ em um guia de canal difundido 1-D e em um guia de canal uniforme. Comparação da equação integral vetorial com o método dos elementos finitos e com o método do índice efetivo “modificado”.](img/fig_04.png)

Figura 4 — Curvas de dispersão para o modo dominante $E^{x}_{11}$ em um guia de canal difundido 1-D e em um guia de canal uniforme. Comparação da equação integral vetorial com o método dos elementos finitos e com o método do índice efetivo “modificado”.

### Leitura operacional da Figura 4

Arquivos operacionais:

- curva A: `data/input/figures/fig_04_curve_A_diffused_1d_eq_integral.json`;
- curva B: `data/input/figures/fig_04_curve_B_uniform_reference.json`.

Parâmetros documentados:

- curva A: $n_1 = 1.0$, $n_3 = 1.44$, $n_{2m} = 1.50$, $a = 2b$;
- curva B: $n_1 = 1.0$, $n_3 = 1.44$, $\bar{n}_2 = 1.48$, $a = 2b$.

As expressões documentais dos eixos da figura são:

$$
X = \left(\frac{2b}{\lambda_0}\right)\left(\bar{n}_2^2 - n_3^2\right)^{1/2},
$$

$$
Y = \frac{\left(\frac{\beta}{k_0}\right)^2 - n_3^2}{\bar{n}_2^2 - n_3^2}.
$$

As marcações visuais observadas na figura são compatíveis com uma malha horizontal de $0$ a $4.0$ em passos de $0.2$ e com uma malha vertical de $0$ a $1.2$ em passos de $0.2$.

As curvas mostradas são:

- curva A: integral equation and finite element;
- curva A: modified effective index;
- curva B: integral equation.

Este conjunto será resolvido por duas execuções separadas. A curva A usa `profile_type = "parabolic_1d"` e o perfil

$$
n_2(x,y)=n_3+\frac{(n_{2m}-n_3)(b^2-y^2)}{b^2}, \qquad 0<y<b,
$$

truncado ao retângulo do núcleo. A curva B usa `profile_type = "homogeneous"` com o índice médio $\bar{n}_2$ como referência uniforme. O objetivo, nesta fase, é produzir duas curvas calibradas e explicitamente relacionadas entre si.

Na Fig. 5, representamos o módulo $|E_y(x,y)|$ do campo elétrico para o modo $E^{y}_{21}$ no guia de canal difundido com difusão parabólica, utilizando uma malha de 50 pontos (10 no eixo $x$ e 5 no eixo $y$), no comprimento de onda de $0.6328\,\mu\text{m}$ (comprimento de onda do laser He-Ne), com dimensão $a=2.22\,\mu\text{m}$.

![Figura 5 — Distribuição de campo do módulo de $E_y(x,y)$ para o modo $E^{y}_{21}$, no comprimento de onda $\lambda_0=0.6328\,\mu\text{m}$, em um guia de canal difundido 1-D com perfil parabólico do índice de refração, com $a=2b=2.22\,\mu\text{m}$.](img/fig_05.png)

Figura 5 — Distribuição de campo do módulo de $E_y(x,y)$ para o modo $E^{y}_{21}$, no comprimento de onda $\lambda_0=0.6328\,\mu\text{m}$, em um guia de canal difundido 1-D com perfil parabólico do índice de refração, com $a=2b=2.22\,\mu\text{m}$.

### Leitura operacional da Figura 5

- arquivo operacional: `data/input/figures/fig_05_field_map_preparation.json`;
- modo-alvo: $E^y_{21}$;
- perfil: `parabolic_1d`;
- parâmetros documentados: $\lambda_0 = 0.6328\,\mu\text{m}$, $\beta/k_0 = 1.4447$, $n_1 = 1.0$, $n_3 = 1.44$, $n_{2m} = 1.5$, $a = 2b = 2.22\,\mu\text{m}$;
- artefatos principais e auxiliares: `field_map.csv`, `field_sampling_grid.csv`, `mode_coefficients.csv`, `field_map_status.txt`;
- saída declarada hoje no manifesto do caso: `field_sampling_grid.csv`.

A Figura 5 mostra uma superfície bidimensional do módulo $|E_y(x,y)|$ em um gráfico 3D. No repositório, o plano de amostragem é explicitado por `field_sampling_grid.csv`, com $x \in [-a/2, a/2]$ e $y \in [0,b]$, enquanto o campo reconstruído permanece gravado em `field_map.csv` e também em `results.csv`.

Este caso será resolvido de forma diferente das curvas de dispersão. Parte-se de $\lambda_0$ e $\beta/k_0$ fixados pelo artigo, monta-se o operador discretizado para o perfil parabólico, estima-se um vetor modal no $\beta$ informado e reconstrói-se a componente pedida em uma grade regular. O resultado desta fase é um mapa de campo auditável, não ainda uma validação modal definitiva do modo $E^y_{21}$.

### 3.3.2. Difusão circular: modo $E^{x}_{11}$

Examina-se aqui o caso de uma forma circular para o índice de refração $n_2(x,y)$. Assim como antes, considera-se uma região de núcleo retangular circundada por um substrato uniforme.

O índice de refração é descrito por

$$
n_2(x,y)=n_3+\frac{(n_{2m}-n_3)}{L^2}(L^2-x^2-y^2)
$$

onde $n_{2m}$ é o índice máximo na origem, $n_3$ é o índice do substrato, e

$$
L^2=b^2+x^2 \quad \text{se } y>x
$$

$$
L^2=\left(\frac{a}{2}\right)^2+y^2 \quad \text{se } y<x
$$

Aqui, $a$ e $b$ são, respectivamente, a largura e a profundidade do núcleo. $L$ é o comprimento da reta que vai da origem até a fronteira.

Também foi realizada a comparação com um guia de canal homogêneo que possui o índice médio $\bar{n}_2$ do guia difundido $(\bar{n}_2=1.47)$, correspondente à curva B da Fig. 6.

![Figura 6 — Curvas de dispersão para o modo dominante $E^{x}_{11}$ em um guia de canal difundido 2-D e em um guia de canal uniforme.](img/fig_06.png)

Figura 6 — Curvas de dispersão para o modo dominante $E^{x}_{11}$ em um guia de canal difundido 2-D e em um guia de canal uniforme.

### Leitura operacional da Figura 6

Arquivos operacionais:

- curva A: `data/input/figures/fig_06_curve_A_diffused_2d_eq_integral.json`;
- curva B: `data/input/figures/fig_06_curve_B_uniform_reference.json`.

Parâmetros documentados:

- curva A: $n_1 = 1.0$, $n_3 = 1.44$, $n_{2m} = 1.50$, $a = 2b$;
- curva B: $n_1 = 1.0$, $n_3 = 1.44$, $\bar{n}_2 = 1.47$, $a = 2b$.

As expressões documentais do perfil difundido circular são:

$$
n_2(x,y)=n_3+\frac{(n_{2m} - n_3)}{L^2}(L^2-x^2-y^2),
$$

$$
L^2=b^2+x^2 \quad \text{se } y \ge x,
$$

$$
L^2=\left(\frac{a}{2}\right)^2+y^2 \quad \text{se } y < x.
$$

As expressões documentais dos eixos da figura são:

$$
X = \left(\frac{2b}{\lambda_0}\right)\left(\bar{n}_2^2 - n_3^2\right)^{1/2},
$$

$$
Y = \frac{\left(\frac{\beta}{k_0}\right)^2 - n_3^2}{\bar{n}_2^2 - n_3^2}.
$$

As marcações visuais observadas na figura são compatíveis com uma malha horizontal de $0$ a $4.0$ em passos de $0.2$ e com uma malha vertical de $0$ a $1.2$ em passos de $0.2$.

As curvas mostradas são:

- curva A: guia difundido circular 2-D;
- curva B: referência uniforme por índice médio.

Este conjunto será resolvido com duas execuções separadas. A curva A usa `profile_type = "circular_2d"` e a curva B usa `profile_type = "homogeneous"` com $\bar{n}_2 = 1.47$. O compromisso da Fase 1 é manter essa distinção documentada e operacional; a fidelidade final do perfil circular continua sendo tema da formulação computacional e da validação posterior.

## 3.4. Casos auxiliares e testes automatizados

### `data/input/smoke_homogeneous.json`

Este caso de sanidade não tenta reproduzir uma figura do artigo. Ele serve para verificar o encadeamento mínimo do executável: leitura do JSON, montagem de malha, varredura modal simples e escrita dos artefatos básicos em um diretório de saída. Ele é resolvido pelo próprio binário `waveguide_solver`, invocado no teste `tests/smoke_case_loading.sh`.

### `data/input/smoke_circular_2d.json`

Este caso de sanidade existe para verificar se o perfil `circular_2d` é lido corretamente, gera malha consistente e produz saídas organizadas, sem depender ainda da comparação científica da Figura 6. Ele é resolvido pelo mesmo fluxo dos demais casos, mas com um intervalo pequeno de varredura usado apenas como prova operacional do caminho de execução.

### `data/input/fig4_parabolic_1d.json`

Este caso permanece como um elo entre a formulação do texto de `3.3.1` e a infraestrutura de execução. Ele será resolvido como uma varredura de dispersão com perfil parabólico 1-D, mas deve ser entendido como caso auxiliar histórico; a referência principal da Figura 4, nesta etapa, já está centralizada nos arquivos em `data/input/figures/`.

### `tests/test_green_function.cpp`

Este teste valida o núcleo de Green usado pelo protótipo atual. Ele é resolvido fora do fluxo de figuras: compila-se um executável de teste dedicado e verifica-se simetria, decaimento, coerência das derivadas em relação a diferenças finitas, equivalência entre o bundle interno de `G_NS` e os wrappers escalares, além da instrumentação de ramos oscilatório e transformado. O objetivo aqui não é reproduzir uma figura, mas garantir que a infraestrutura matemática elementar usada pela montagem permaneça auditável.

### `tests/test_matrix_solver.cpp`

Este teste valida a montagem e a busca modal do protótipo em matrizes pequenas. Ele é resolvido montando operadores para perfis homogêneos e parabólicos, conferindo desacoplamentos esperados, presença de acoplamentos regulares e de fronteira, diferença entre modelos de quadratura, finitude dos coeficientes modais encontrados, coerência da média singular log-aware de $G^S$ e contagem dos caminhos compartilhados de `G_NS`. É o principal teste estrutural do solver nesta etapa.

### `tests/smoke_case_loading.sh`

Este script é o teste de fumaça mais simples do repositório. Ele será resolvido compilando o projeto, executando o caso `smoke_homogeneous` em um diretório temporário e verificando a presença de `results.csv`, `profile_samples.csv`, `input_snapshot.json`, `run_summary.txt`, `output_manifest.json` e `performance_summary.json`, além dos cabeçalhos esperados.

### `tests/smoke_figures_csv_generation.sh`

Este script é o teste de fumaça do lote principal das figuras. Ele será resolvido gerando as saídas das Figuras 2 a 6 a partir do `manifest_figures_2_to_6.csv`, executando novamente a Figura 5 pelo caminho padrão de `out/figures/` e checando tanto os arquivos esperados quanto os cabeçalhos, os status registrados nos manifests de saída e os novos artefatos `performance_summary.json`.

### `tests/smoke_benchmark_figures_2_to_6.sh`

Este script é o teste de fumaça do benchmark oficial da Fase 3.1A. Ele será resolvido gerando um manifest reduzido das Figuras 2 a 6, executando `scripts/benchmark_figures_2_to_6.sh` e verificando a presença de `benchmark_cases.csv`, `benchmark_summary.json` e dos `performance_summary.json` esperados nas saídas por caso.

## 3.5. Leitura de fechamento da Fase 1

Depois desta consolidação, a Fase 1 fica documentada assim:

- o problema físico já está traduzido e separado da discretização;
- a notação já está consolidada em um dicionário de símbolos;
- as figuras-alvo já estão mapeadas para arquivos de entrada concretos;
- cada figura de reprodução já tem uma descrição explícita de como será resolvida;
- os casos auxiliares e os testes automatizados já estão ligados ao seu papel dentro do repositório.

O que ainda resta aberto já não é uma lacuna de definição documental do problema. É trabalho das fases seguintes: fechamento da formulação vetorial, robustez da busca modal, convergência de malha e validação quantitativa fina.

## 3.6. Nota editorial de consolidação

As informações que antes apareciam separadas em `docs/03.3_exemplos.md` e `docs/09_figuras.md` passam a ser mantidas aqui. Os dois arquivos antigos continuam no repositório apenas para preservar backlinks e registrar a consolidação documental.

---
**Navegação:** [00 Resumo](00_titulo_autoria_resumo.md) | [01 Introdução](01_introducao.md) | [02 Formulação](02_formulacao_do_problema_de_valor_de_contorno.md) | [02 Símbolos](02_symbol_dictionary.md) | [02 Teoria](02_teoria.md) | [03 Resultados](03_resultados_numericos.md) | [04 Conclusões](04_conclusoes.md) | [05 Referências](05_referencias.md) | [06 Auditoria](06_auditoria_inicial_do_repositorio.md) | [12 Trilha do Código](12_trilha_equacoes_para_codigo.md) | [Plano](../PLAN.md) | [TODO](../TODO.md)
