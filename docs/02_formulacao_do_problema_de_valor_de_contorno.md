# 02. Formulação do problema de valor de contorno

Este documento não substitui `docs/02_teoria.md`.

Seu papel é fixar, de forma auditável, a formulação física mínima que o repositório precisa preservar antes da discretização: geometria, domínios, variáveis, perfis de índice e convenções de sinal.

## 1. Problema físico que o artigo resolve

O artigo estuda modos guiados em um **diffused channel waveguide** dielétrico embutido em um meio estratificado.

Há três regiões:

- $\mathcal{D}_1$: superstrato homogêneo;
- $\mathcal{D}_2$: região do guia, onde o índice pode ser homogêneo ou variar com $(x,y)$;
- $\mathcal{D}_3$: substrato homogêneo.

O problema físico consiste em determinar, para um comprimento de onda no vácuo $\lambda_0$, os valores admissíveis da constante de propagação $\beta$ e a distribuição transversal do campo elétrico $\mathbf{E}(x,y)$ de modos guiados híbridos que se propagam ao longo de $z$.

## 2. Convenção geométrica adotada no repositório

Para a implementação e para os arquivos de entrada já existentes, este repositório adota a seguinte convenção operacional:

- eixo $z$: direção de propagação;
- eixo $x$: direção lateral do canal;
- eixo $y$: profundidade medida a partir da superfície, positiva para dentro do substrato;
- interface ar/substrato: $y=0$;
- superstrato: $y<0$;
- substrato: $y>0$;
- suporte geométrico do guia: $|x| \le a/2$ e $0 \le y \le b$.

Assim, a região de integração $\mathcal{D}_2$ é o retângulo

$$
\mathcal{D}_2 = \left\{(x,y)\; ;\; -\frac{a}{2} \le x \le \frac{a}{2},\; 0 \le y \le b \right\},
$$

e sua fronteira é denotada por $\mathcal{R} = \partial \mathcal{D}_2$.

Essa convenção é coerente com:

- os perfis escritos em `docs/03_resultados_numericos.md`, que usam $0<y<b$;
- os casos em `data/input/figures/`;
- a implementação atual em `src/waveguide.cpp`.

Nota editorial:
`docs/02_teoria.md` foi mantido fiel à redação e à convenção geométrica do artigo original. O código e os casos executáveis deste repositório, por sua vez, adotam uma convenção operacional em que $y>0$ aponta para dentro do substrato. A divergência, portanto, já não é tratada como ambiguidade editorial em aberto: ela é uma escolha explícita de implementação que precisa permanecer rastreável.

## 3. Ansatz modal e grandezas eletromagnéticas

O artigo trabalha com dependência temporal harmônica

$$
e^{-i\omega t}
$$

e separação modal ao longo de $z$:

$$
\mathbf{E}(x,y,z)=\mathbf{E}(x,y)e^{i\beta z}.
$$

As grandezas materiais e espectrais mínimas da formulação são:

$$
k_0=\frac{2\pi}{\lambda_0},
\qquad
k_j = k_0 n_j,
\qquad
\varepsilon_j = \varepsilon_0 n_j^2,
\qquad
\mu=\mu_0.
$$

Na região do guia, o artigo também usa $k_2(x,y)$ e $\varepsilon_2(x,y)$ para indicar o perfil espacialmente variável.

O núcleo do problema é encontrar $\beta$ e o campo transversal associados a modos guiados. Na formulação do artigo, isso é reescrito como uma equação integral vetorial sobre $\mathcal{D}_2$, com uma função de Green do meio estratificado de referência.

## 4. Domínios, contraste de índice e função incógnita

Nesta etapa, o problema de valor de contorno pode ser resumido assim:

1. fixa-se a geometria $(a,b)$ e o perfil material;
2. fixa-se $\lambda_0$;
3. define-se um meio de fundo estratificado com superstrato homogêneo $n_1$ e substrato homogêneo $n_3$;
4. descreve-se o guia por uma inomogeneidade confinada a $\mathcal{D}_2$;
5. busca-se $\beta$ tal que a equação integral vetorial admita solução modal não trivial.

A função incógnita central é o campo elétrico transversal no guia, tratado pelo artigo como um campo vetorial híbrido. Na prática computacional atual do repositório, os componentes $E_x$ e $E_y$ são os graus de liberdade que organizam a montagem.

## 5. Perfis de índice que entram na Fase 1

Esta fase não implementa toda a física final, mas fixa com precisão quais perfis o projeto precisa sustentar documentalmente.

### 5.1. Perfil homogêneo

Usado nas Figuras 2 e 3 e também como curva de referência uniforme nas Figuras 4 e 6.

Dentro de $\mathcal{D}_2$:

$$
n_2(x,y)=n_{2m}.
$$

Fora de $\mathcal{D}_2$:

$$
n(x,y)=
\begin{cases}
n_1, & y<0,\\
n_3, & y \ge 0 \text{ e } (x,y)\notin\mathcal{D}_2.
\end{cases}
$$

Observação:
nos casos homogêneos do repositório, a chave `n2m` também é usada para representar o índice constante do núcleo.

### 5.2. Perfil difundido parabólico 1-D

Usado nas Figuras 4 e 5.

Dentro de $\mathcal{D}_2$:

$$
n_2(x,y)=n_3+\frac{(n_{2m}-n_3)(b^2-y^2)}{b^2},
\qquad 0<y<b.
$$

Esse perfil varia apenas com a profundidade $y$ e permanece truncado ao retângulo do núcleo.

### 5.3. Perfil difundido circular 2-D

Usado na Figura 6.

Dentro de $\mathcal{D}_2$, o texto traduzido em `docs/03_resultados_numericos.md` e a implementação atual usam

$$
n_2(x,y)=n_3+\frac{(n_{2m}-n_3)}{L^2(x,y)}\left(L^2(x,y)-x^2-y^2\right),
$$

com

$$
L^2(x,y)=
\begin{cases}
b^2+x^2, & y>x,\\
\left(\dfrac{a}{2}\right)^2+y^2, & y \le x.
\end{cases}
$$

Nota editorial:
essa escrita acompanha a forma hoje registrada em `docs/02_teoria.md`, `docs/03_resultados_numericos.md` e nos arquivos operacionais da Figura 6. Ela deve ser mantida de forma consistente sempre que a formulação da Figura 6 for recalibrada.

### 5.4. Perfis uniformes de referência por índice médio

As Figuras 4 e 6 não comparam apenas o guia difundido com a solução integral. Elas também incluem uma curva uniforme de referência com índice médio $\bar{n}_2$:

- Figura 4: $\bar{n}_2 = 1.48$;
- Figura 6: $\bar{n}_2 = 1.47$.

No repositório, esses casos entram como perfis homogêneos distintos, com `profile_type = "homogeneous"` e `n2m = \bar{n}_2`.

## 6. Grandezas mínimas que cada caso precisa declarar

Para a Fase 1, todo caso de reprodução precisa declarar explicitamente:

- figura e seção do artigo;
- modo alvo;
- $n_1$, $n_3$ e o parâmetro de núcleo correspondente (`n2m` ou $\bar{n}_2$);
- dimensões $a$ e $b$;
- tipo de perfil (`homogeneous`, `parabolic_1d` ou `circular_2d`);
- tipo de estudo: curva de dispersão ou mapa de campo.

Essas convenções já aparecem em `data/input/figures/` e passam a ter, com este documento, uma interpretação física consolidada.

## 7. Figuras efetivamente visadas pela reprodução

O papel das figuras fica assim separado:

- Figura 1: geometria conceitual e convenção de domínios; não é um alvo numérico independente;
- Figuras 2, 3, 4 e 6: alvos de curvas de dispersão;
- Figura 5: alvo de reconstrução de campo.

O detalhamento figura por figura está em `docs/03.3_exemplos.md`.
