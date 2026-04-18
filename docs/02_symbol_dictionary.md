# 02. Dicionário de símbolos

Este arquivo consolida a notação usada pelo artigo, pela documentação traduzida e pelos arquivos de entrada do repositório.

Quando a implementação usa um nome operacional diferente da notação matemática, a equivalência fica registrada explicitamente.

## 1. Geometria e domínios

| Símbolo | Significado | Uso no repositório |
| --- | --- | --- |
| $\mathcal{D}_1$ | superstrato homogêneo | região com índice `n1` |
| $\mathcal{D}_2$ | região do guia | domínio de integração e discretização |
| $\mathcal{D}_3$ | substrato homogêneo | região com índice `n3` |
| $\mathcal{R}$ | fronteira de $\mathcal{D}_2$ | borda usada no termo distribucional |
| $x$ | coordenada lateral transversal | largura do canal |
| $y$ | coordenada transversal vertical | profundidade; nesta etapa, $y>0$ aponta para dentro do substrato |
| $z$ | coordenada longitudinal | direção de propagação |
| $a$ | largura do guia | chave `geometry.a` |
| $b$ | profundidade do guia | chave `geometry.b` |
| $L$ | distância até a fronteira no perfil circular | usada na Figura 6 |

## 2. Materiais e ondas

| Símbolo | Significado | Uso no repositório |
| --- | --- | --- |
| $\varepsilon_0$ | permissividade do vácuo | constante física |
| $\mu_0$ | permeabilidade do vácuo | constante física |
| $\varepsilon_j$ | permissividade do meio $j$ | associada a $n_j$ por $\varepsilon_j=\varepsilon_0 n_j^2$ |
| $\varepsilon_2(x,y)$ | permissividade variável no guia | equivalente ao perfil espacial do núcleo |
| $n_1$ | índice do superstrato | chave `materials.n1` |
| $n_3$ | índice do substrato | chave `materials.n3` |
| $n_2(x,y)$ | índice no guia | perfil homogêneo ou difundido |
| $n_{2m}$ | índice máximo do guia | chave `materials.n2m` |
| $\bar{n}_2$ | índice médio do guia difundido usado como referência uniforme | também entra em `materials.n2m` nos casos homogêneos equivalentes |
| $\lambda_0$ | comprimento de onda no vácuo | chave `field_map.lambda0` ou variável derivada da abcissa do artigo |
| $k_0$ | número de onda no vácuo | $2\pi/\lambda_0$ |
| $k_j$ | número de onda no meio $j$ | $k_0 n_j$ |
| $k_2(x,y)$ | número de onda no guia | $k_0 n_2(x,y)$ |
| $\omega$ | frequência angular | aparece no ansatz temporal |
| $\beta$ | constante de propagação modal | incógnita espectral principal |

## 3. Campos e formulação modal

| Símbolo | Significado | Uso no repositório |
| --- | --- | --- |
| $\mathbf{E}(x,y,z)$ | campo elétrico total | grandeza física do artigo |
| $\mathbf{E}(x,y)$ | campo transversal modal | incógnita após separação em $z$ |
| $E_x$, $E_y$ | componentes transversais do campo | graus de liberdade principais da montagem atual |
| $E^x_{pq}$ | família modal com estrutura dominante em $x$ | notação do artigo |
| $E^y_{pq}$ | família modal com estrutura dominante em $y$ | notação do artigo |
| $p$, $q$ | contagem modal nas direções $x$ e $y$ | usados nas legendas das figuras |
| $G$ | função de Green do meio estratificado de referência | kernel da equação integral |
| $G_S$ | parte singular de $G$ | já implementada |
| $G_{NS}$ | parte não singular de $G$ | já implementada |
| $\gamma_1$, $\gamma_3$ | parâmetros espectrais da função de Green | aparecem na Eq. (6) |
| $\nu$ | variável de Fourier transversal | aparece na representação integral de $G$ |
| $\sigma_{1/\varepsilon}$ | salto de $1/\varepsilon$ na fronteira | aparece na forma distribucional da Eq. (4) em `docs/02_teoria.md` |

## 4. Discretização e saída numérica

| Símbolo ou nome | Significado | Uso no repositório |
| --- | --- | --- |
| $N_x$, $N_y$ | número de células/pontos por direção | chaves `discretization.Nx` e `discretization.Ny` |
| $A$ | matriz do sistema do método dos momentos | operador discretizado |
| $\det(A)$ | determinante do sistema discretizado | residual espectral usado na busca modal atual |
| `modal_residual` | residual do vetor quase-nulo | métrica adicional de seleção modal |
| `profile_type` | tipo de perfil material | `homogeneous`, `parabolic_1d`, `circular_2d` |
| `study.kind` | tipo de estudo | `dispersion_curve` ou `field_map` |
| `beta_over_k0` | $\beta/k_0$ | coluna de saída e parâmetro do mapa de campo |
| `normalized_beta` | ordenada normalizada da curva | usada para comparar com as figuras do artigo |
| `article_x_param` | abcissa normalizada do artigo | nas curvas, corresponde a $\dfrac{2b}{\lambda_0}\sqrt{n_\text{alto}^2-n_\text{baixo}^2}$ |

## 5. Convenções de nomenclatura que precisam permanecer explícitas

- `n2m` não significa sempre "índice máximo de um perfil difundido". Nos casos homogêneos, ele também representa o índice constante do núcleo.
- `profile_type = "homogeneous"` pode significar tanto um caso-base homogêneo do artigo quanto uma curva de referência uniforme com índice médio.
- `beta_over_k0` é uma quantidade de saída para curvas e também uma entrada explícita para a reconstrução de campo da Figura 5.
- a notação modal do artigo ($E^x_{pq}$, $E^y_{pq}$) não deve ser substituída por rótulos genéricos sem preservar o modo-alvo associado a cada figura.
- `docs/02_teoria.md` preserva a convenção geométrica do artigo, enquanto a implementação usa uma convenção operacional própria para o sinal de $y$; essa diferença precisa continuar explicitada, nunca escondida.

---
**Navegação:** [00 Resumo](00_titulo_autoria_resumo.md) | [01 Introdução](01_introducao.md) | [02 Formulação](02_formulacao_do_problema_de_valor_de_contorno.md) | [02 Símbolos](02_symbol_dictionary.md) | [02 Teoria](02_teoria.md) | [03 Resultados](03_resultados_numericos.md) | [04 Conclusões](04_conclusoes.md) | [05 Referências](05_referencias.md) | [06 Auditoria](06_auditoria_inicial_do_repositorio.md) | [12 Trilha do Código](12_trilha_equacoes_para_codigo.md) | [Plano](../PLAN.md) | [TODO](../TODO.md)
