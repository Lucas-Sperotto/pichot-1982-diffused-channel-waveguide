# 12. Trilha das equações para o código

Este documento abre a Fase 2.

Seu objetivo é transformar a formulação do artigo em uma sequência computacional auditável, deixando explícito:

- quais passos do método dos momentos já aparecem no protótipo;
- quais aproximações operacionais foram adotadas para manter o código executável;
- quais pontos ainda precisam de fechamento científico para dizer que a implementação reproduz integralmente a formulação do artigo.

O ponto de partida continua sendo `docs/02_teoria.md`, especialmente as Eqs. (1) a (8). Aqui a ênfase deixa de ser apenas física e passa a ser algorítmica.

## 12.1. Entradas, incógnitas e objetos computacionais

No artigo, a incógnita principal é a constante de propagação $\beta$, juntamente com o campo transversal híbrido $\mathbf{E}(x,y)$.

No repositório atual, isso é representado por:

- `SimulationCase`, carregado de `data/input/...`, com geometria, materiais, discretização, tipo de estudo e opções de montagem;
- `WaveguideParams`, que fixa $n_1$, $n_3$, $\bar{n}_2$, $a$, $b$, $\lambda_0$ e o tipo de perfil;
- `Discretization`, que fixa `Nx` e `Ny`;
- `AssemblyOptions`, que liga ou desliga os blocos operacionais da montagem;
- `ModeSolution`, que guarda $\beta$, `determinant_magnitude`, `modal_residual` e o vetor modal discretizado.

O campo contínuo $\mathbf{E}(x,y)$ ainda não é representado por funções analíticas. Ele é representado por coeficientes discretos constantes por célula, em coerência com a afirmação do artigo de que foram usadas funções-base do tipo step.

## 12.2. Discretização da região do guia

A equação integral do artigo integra apenas sobre $\mathcal{D}_2$, isto é, a região do guia. O protótipo atual preserva exatamente essa ideia estrutural: a malha é construída apenas dentro do domínio do guia, não nos meios externos.

Na convenção operacional do código:

- o guia ocupa o retângulo $[-a/2,a/2] \times [0,b]$;
- a malha é uniforme, com `Nx * Ny` células retangulares;
- cada célula $\Omega_j$ guarda centro $(x_j,y_j)$, larguras `dx`, `dy` e área $|\Omega_j| = dx\,dy$.

Em `src/waveguide.cpp`, a discretização produz dois conjuntos de dados:

- `cells`, usados como células-fonte e células de observação;
- `boundary_segments`, usados para isolar a contribuição distributiva da Eq. (4) ao longo da fronteira $\mathcal{R}$.

Do ponto de vista do método dos momentos, a interpretação operacional atual é:

- cada célula carrega uma função-base step para $E_x$;
- a mesma célula carrega uma função-base step para $E_y$;
- os testes são feitos por colocação no centro das células.

Isso leva ao vetor discreto

$$
X =
\begin{bmatrix}
E_{x,1} & \cdots & E_{x,N} & E_{y,1} & \cdots & E_{y,N}
\end{bmatrix}^{T},
$$

com $N = N_x N_y$.

## 12.3. Perfis de índice e termos locais

As Eqs. (1) e (2) entram no código por meio das funções de perfil e de gradiente regular:

- `n_homogeneous`;
- `n_parabolic_1d`;
- `n_circular_2d`;
- `grad_n_homogeneous`;
- `grad_n_parabolic_1d`;
- `grad_n_circular_2d`.

A partir delas, o protótipo calcula:

- $k^2(x,y) = k_0^2 n^2(x,y)$, via `Waveguide::get_k_squared`;
- a parte regular de $\varepsilon \,\mathrm{grad}(1/\varepsilon)$, via `Waveguide::get_regular_epsilon_grad_inverse`.

O salto de $1/\varepsilon$ sobre a fronteira, que no artigo aparece na Eq. (4) por meio de $\sigma_{1/\varepsilon}\,\delta_{\mathcal{R}}$, é traduzido operacionalmente por segmentos de fronteira com:

- normal exterior explícita;
- índices interno e externo amostrados por sondagem local;
- fator `epsilon_jump_factor`.

Assim, o protótipo já separa, no nível computacional:

- a contribuição volumétrica regular;
- a contribuição singular/distributiva concentrada na fronteira.

## 12.4. Avaliação do núcleo de Green

As Eqs. (5), (6) e (7) são implementadas em `src/green_function.cpp`.

O escopo atual é deliberadamente restrito:

- apenas o regime guiado é aceito;
- exige-se $\beta^2 > k_1^2$ e $\beta^2 > k_3^2$;
- a implementação cobre apenas pontos com $y \ge 0$ e $y' \ge 0$, isto é, a parte do problema que o protótipo hoje realmente usa ao integrar sobre as células do guia.

O núcleo total é escrito como

$$
G = G_S + G_{NS}.
$$

O tratamento computacional atual é:

- `calculate_G_S`: usa a forma fechada com a função de Bessel modificada $K_0$;
- `calculate_G_NS`: avalia a integral de Fourier sobre $\nu \in [0,+\infty)$ após a transformação $\nu = t/(1-t)$, seguida de quadratura adaptativa de Simpson em $t \in [0,1)$;
- `calculate_dG_S_dx_source` e `calculate_dG_S_dy_source`: usam derivadas analíticas da parte singular com $K_1$;
- `calculate_dG_NS_dx_source` e `calculate_dG_NS_dy_source`: usam diferenças finitas do próprio `G_NS`.

Isto preserva a decomposição do artigo, mas ainda não fecha toda a parte numérica com o mesmo rigor teórico do texto original. Em particular:

- as derivadas da parte não singular ainda não usam uma fórmula integral dedicada;
- a singularidade logarítmica de $G_S$ ainda não recebe uma quadratura analítica exata ao nível de célula.

## 12.5. Montagem da matriz do método dos momentos

A Eq. (8),

$$
A X = 0,
$$

é montada em `src/matrix_solver.cpp`.

Para cada par observação-fonte $(i,j)$, o protótipo constrói quatro blocos escalares:

$$
A_{xx}^{(ij)}, \quad A_{xy}^{(ij)}, \quad A_{yx}^{(ij)}, \quad A_{yy}^{(ij)}.
$$

Com $q_j = \varepsilon \,\mathrm{grad}(1/\varepsilon)$ avaliado no centro da célula-fonte e $\overline{G}_{ij}$ denotando a avaliação operacional do kernel entre as células, a montagem atual pode ser lida como:

$$
A_{xx}^{(ij)} =
\delta_{ij}
- |\Omega_j| \Delta k_j^2 \,\overline{G}_{ij}
- |\Omega_j| q_{x,j}\,\overline{\partial_{x'} G}_{ij}
- B_{xx}^{(ij)},
$$

$$
A_{xy}^{(ij)} =
- |\Omega_j| q_{y,j}\,\overline{\partial_{x'} G}_{ij}
- B_{xy}^{(ij)},
$$

$$
A_{yx}^{(ij)} =
- |\Omega_j| q_{x,j}\,\overline{\partial_{y'} G}_{ij}
- B_{yx}^{(ij)},
$$

$$
A_{yy}^{(ij)} =
\delta_{ij}
- |\Omega_j| \Delta k_j^2 \,\overline{G}_{ij}
- |\Omega_j| q_{y,j}\,\overline{\partial_{y'} G}_{ij}
- B_{yy}^{(ij)},
$$

onde:

- $\Delta k_j^2 = k^2(x_j,y_j) - k_3^2$;
- $B_{\alpha\beta}^{(ij)}$ representa a contribuição explícita da fronteira;
- $\delta_{ij}$ entra como identidade do operador integral reescrito em forma residual.

Os blocos da matriz são então armazenados na ordem:

- canto superior esquerdo: `A_xx`;
- canto superior direito: `A_xy`;
- canto inferior esquerdo: `A_yx`;
- canto inferior direito: `A_yy`.

Na implementação atual, essa montagem já foi separada explicitamente em quatro contribuições de código:

- identidade residual;
- termo volumétrico escalar;
- termo regular de gradiente;
- termo distributivo de fronteira.

Essa separação é importante porque permite trocar, no futuro, o método numérico de cada bloco sem reescrever a montagem inteira.

### 12.5.1. Termo volumétrico escalar

O primeiro termo da Eq. (3) é aproximado por célula-fonte constante:

- avalia-se $k^2-k_3^2$ no centro da célula-fonte;
- multiplica-se pela área da célula;
- multiplica-se pela avaliação operacional do kernel de Green entre fonte e observação.

O método numérico escolhido para este bloco é, portanto, uma aproximação peça-por-peça constante no domínio da célula, coerente com funções-base step e colocação no centro.

### 12.5.2. Termo regular de gradiente

A parte regular da Eq. (4) entra via `get_regular_epsilon_grad_inverse`. No protótipo atual:

- o gradiente regular é avaliado no centro da célula-fonte;
- ele acopla $E_x$ e $E_y$ aos termos com $\partial_{x'}G$ e $\partial_{y'}G$;
- essa contribuição já é vetorial, mas ainda representa uma tradução operacional simplificada da formulação integral do artigo.

O método numérico adotado aqui também é peça-por-peça constante por célula-fonte, usando derivadas do kernel avaliadas operacionalmente entre células.

### 12.5.3. Termo distributivo de fronteira

A parte singular da Eq. (4) é tratada fora da integral volumétrica:

- cada célula de borda gera segmentos em `boundary_segments`;
- cada segmento carrega normal exterior e `epsilon_jump_factor`;
- a integral ao longo de cada segmento pode usar `midpoint` ou `gauss2`;
- `boundary_subdivisions` refina a integração por subsegmentos.

Esse é um passo importante da Fase 2 porque torna auditável, em código, a presença da distribuição concentrada em $\mathcal{R}$, em vez de escondê-la dentro de um gradiente regularizado informal.

O método numérico escolhido para este bloco é uma quadratura de linha por segmentos explícitos, com regra do ponto médio ou Gauss de dois pontos.

### 12.5.4. Auto-interação e regularização local

Quando a célula de observação coincide com a célula-fonte, o termo singular de $G_S$ não pode ser avaliado no mesmo ponto. O protótipo atual resolve isso por uma média em quatro subpontos internos da célula-fonte.

Esta escolha deve ser lida corretamente:

- ela evita infinito numérico e mantém a matriz finita;
- ela preserva a rastreabilidade do operador;
- ela não deve ser confundida com o tratamento singular definitivo do artigo.

Na implementação mais recente do protótipo, essa etapa já foi melhorada:

- a média do próprio kernel na célula singular é feita por quadratura de Gauss 2x2 em subcélulas;
- a média de $\partial_{x'}G$ na auto-interação é anulada por simetria da célula step em torno do centro;
- a média de $\partial_{y'}G$ na auto-interação integra apenas a parte regular associada a $G_{NS}$, porque a parte singular de $G_S$ se anula por simetria.

Esse tratamento ainda é uma regularização operacional, não a avaliação singular final do artigo, mas já é mais coerente com a interpretação de média sobre célula do que a antiga amostragem em quatro pontos.

## 12.6. Critério computacional para encontrar $\beta$

O artigo afirma que os modos correspondem aos zeros de $\det(A)$.

O protótipo atual implementa isso em duas etapas operacionais:

1. `find_beta_root` procura um candidato a $\beta$ priorizando a quase-nulidade do operador, medida por `modal_residual`, em uma sequência de grades refinadas dentro do intervalo guiado;
2. `refine_beta_with_modal_residual` reavalia uma vizinhança do candidato com o mesmo critério, mantendo $|\det(A)|$ como diagnóstico auxiliar.

$$
\frac{\|AX\|}{\|X\|}.
$$

O intervalo de busca hoje é tomado como

$$
k_0 n_3 \le \beta \le k_0 \bar{n}_2.
$$

Na prática, isso significa que o código atual ainda não executa um localizador rigoroso de zeros exatos de $\det(A)$. Ele usa:

- uma busca inicial guiada por quase-nulidade do operador;
- $|\det(A)|$ como número auxiliar para comparar amostras e inspecionar diagnósticos.

Esse compromisso já é suficiente para os testes de fumaça e para gerar curvas exploratórias, mas permanece uma pendência central da Fase 2.

## 12.7. Como o vetor modal é estimado

Uma vez escolhido $\beta$, `solve_mode_at_beta`:

- monta a matriz `A`;
- calcula `determinant_magnitude`;
- constrói a matriz normal $A^{*}A$;
- adiciona uma regularização diagonal pequena;
- aplica iteração inversa para estimar o vetor singular associado à menor singularidade.

Esse vetor é usado como estimativa do modo discreto e armazenado em `ModeSolution::coefficients`.

Portanto, a regra operacional do repositório hoje é:

- $\beta$ é estimado pela quase-anulação de `A`;
- $X$ é estimado pelo vetor quase-nulo correspondente.

## 12.8. Curvas de dispersão e mapas de campo

Em `src/main.cpp`, a trilha algorítmica se divide em dois fluxos.

### 12.8.1. Casos `dispersion_curve`

Para curvas de dispersão:

- lê-se a abcissa do artigo a partir de `sweep`;
- converte-se essa abcissa em $\lambda_0$ pela fórmula operacional atualmente usada no repositório,

$$
\lambda_0 = \frac{2 b \sqrt{\bar{n}_2^2 - n_3^2}}{\text{article\_x\_param}};
$$

- reconstrói-se `Waveguide`;
- procura-se $\beta$;
- calcula-se `normalized_beta`, `det_abs` e `modal_residual`;
- grava-se `dispersion_curve.csv`.

Como as funções-base são do tipo step, o aumento de `Nx` e `Ny` é a forma direta de refinar a aproximação espacial. A conferência de que essa conversão de abcissa coincide exatamente com cada figura do artigo permanece parte da validação científica posterior.

### 12.8.2. Casos `field_map`

Para mapas de campo:

- fixa-se $\lambda_0$;
- usa-se `beta_over_k0` informado, quando o caso já vem calibrado pelo artigo, ou busca-se $\beta$ automaticamente;
- resolve-se o vetor modal;
- normaliza-se a componente pedida;
- amostra-se a solução em uma grade regular;
- grava-se `field_map.csv`, `field_sampling_grid.csv` e `mode_coefficients.csv`.

Como a base é step, a reconstrução espacial atual é peça-por-peça constante em cada célula. Isso é coerente com a escolha de base, mas também explica por que mapas de campo ainda devem ser lidos como aproximações discretas, não como reconstruções suaves finais.

## 12.9. Mapa de rastreabilidade artigo -> código

| Bloco do artigo | Papel matemático | Tradução computacional atual |
| --- | --- | --- |
| Eq. (1) e Eq. (2) | equação diferencial e definição de $k^2$ | `src/waveguide.cpp`, perfis de índice e `get_k_squared` |
| Eq. (3) | equação integral vetorial no domínio $\mathcal{D}_2$ | `build_matrix_A` em `src/matrix_solver.cpp` |
| Eq. (4) parte regular | $\varepsilon \,\mathrm{grad}(1/\varepsilon)$ no volume | `get_regular_epsilon_grad_inverse` |
| Eq. (4) parte singular | salto de $1/\varepsilon$ em $\mathcal{R}$ | `boundary_segments` + integração de fronteira |
| Eq. (5), Eq. (6), Eq. (7) | função de Green do meio estratificado | `src/green_function.cpp` |
| Eq. (8) | sistema homogêneo do método dos momentos | `ComplexMatrix`, `build_matrix_A`, `ModeSolution` |
| zeros de $\det(A)$ | critério modal em $\beta$ | `find_beta_root` e `refine_beta_with_modal_residual` |
| reconstrução de campo | obtenção de $\mathbf{E}(x,y)$ a partir de $X$ | `write_mode_coefficients` e `write_field_map_grid` |

## 12.10. O que ainda falta para fechar a Fase 2

Os próximos fechamentos desejáveis já ficaram claros a partir desta trilha:

- confirmar, no nível do artigo, se a etapa de teste é melhor descrita como colocação direta nos centros ou se há outro funcional de teste implícito;
- substituir a regularização local de quatro subpontos por um tratamento singular mais rigoroso;
- substituir as diferenças finitas de `G_NS` por derivadas numericamente mais robustas;
- revisar se a tradução vetorial dos blocos cruzados `A_xy` e `A_yx` coincide integralmente com a formulação do artigo;
- trocar a busca por mínimo de $|\det(A)|$ por um critério modal mais rigoroso;
- documentar, figura por figura, a convergência em malha exigida para chamar a reprodução de quantitativamente fechada.

Em outras palavras, a Fase 2 já pode ser trabalhada com base auditável: a sequência algorítmica principal está exposta. O que falta agora não é mais descobrir "como o código anda", mas melhorar a fidelidade matemática de cada etapa.
