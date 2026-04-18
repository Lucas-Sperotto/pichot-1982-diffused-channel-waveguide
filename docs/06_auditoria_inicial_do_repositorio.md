# 06. Auditoria inicial do repositório

Este documento registra a validação inicial de `docs/` e das principais divergências entre a documentação, a estrutura real do repositório e o estado atual do código.

O objetivo aqui não é reescrever a base documental existente, mas tornar explícito o que já está consistente, o que ainda é apenas estruturalmente desejado e quais pendências precisam permanecer auditáveis.

## 1. O que já está consistente em `docs/`

- `docs/00_titulo_autoria_resumo.md` identifica corretamente o artigo, autoria, periódico e escopo geral.
- `docs/01_introducao.md` resume o posicionamento histórico do método e dos trabalhos comparados.
- `docs/02_teoria.md` registra a formulação de Helmholtz, a equação integral vetorial, a função de Green e o papel do método dos momentos.
- `docs/02_formulacao_do_problema_de_valor_de_contorno.md` fixa a convenção geométrica operacional, os domínios e os perfis de índice que o repositório realmente usa.
- `docs/02_symbol_dictionary.md` consolida a notação do artigo e sua equivalência com os nomes operacionais do código e dos arquivos JSON.
- `docs/03_resultados_numericos.md` separa os principais blocos de validação numérica por família de caso.
- `docs/03.3_exemplos.md` fixa quais figuras entram no escopo de reprodução e qual arquivo de entrada representa cada uma.
- `docs/09_figuras.md` registra a consolidação editorial das figuras dentro de `docs/03_resultados_numericos.md`, reduzindo duplicação entre documentos.
- `docs/04_conclusoes.md` e `docs/05_referencias.md` preservam o fechamento científico do artigo.

Em outras palavras, `docs/` já não cobre apenas o arco narrativo do artigo: ao final desta revisão, também cobre a fundação documental mínima exigida para rastrear a implementação da Fase 1.

## 2. Divergências encontradas

### 2.1 Estrutura documentada versus estrutura real

O arquivo `ESTRUTURA.md` descreve uma árvore-alvo com `src/`, `include/`, `cases/`, `tests/` e `results/`.

Na data desta revisão, a divergência inicial diminuiu substancialmente: `src/`, `tests/`, `docs/`, `data/` e `scripts/` já existem e já organizam o trabalho real. O que permanece é que `ESTRUTURA.md` ainda deve ser lido mais como direção arquitetural do que como fotografia exata do repositório, especialmente nos pontos em que cita `include/`, `cases/` e `results/`.

### 2.2 Build script fora de sincronia

O `build_and_run.sh` original apontava para arquivos em `src/` e `include/` que ainda não existiam no repositório. Isso criava uma divergência direta entre a interface de execução e o conteúdo realmente disponível.

Na leitura atual, essa divergência já foi superada pelo fluxo baseado em `scripts/build.sh`, `scripts/run_case.sh` e `scripts/generate_figures_2_to_6_csvs.sh`.

### 2.3 Casos ainda não separados em entradas reproduzíveis

Até esta auditoria, o caso parabólico 1-D estava embutido diretamente em `main.cpp`, o que contrariava a diretriz do projeto de usar arquivos de entrada reproduzíveis.

Na leitura atual, essa pendência também já foi resolvida: os casos operacionais passaram a viver em arquivos JSON, com destaque para `data/input/figures/`.

### 2.4 Núcleo numérico ainda em estágio de protótipo

Os seguintes blocos continuam explicitamente pendentes no código:

- montagem efetiva dos blocos da matriz do método dos momentos;
- busca rigorosa de raízes de `det(A) = 0`.

Atualização desta auditoria:

- a Green escalar `G^S + G^NS` para o regime guiado com `y >= 0` e `y' >= 0` já foi implementada;
- os gradientes de `G^S` já foram implementados em forma fechada;
- os gradientes de `G^NS` estão disponíveis nesta etapa por diferença central do próprio kernel.
- a matriz usada pelo executável já não é mais identidade pura: existe agora um operador escalar protótipo baseado apenas no termo `(k^2-k_3^2)G`, replicado nos blocos `E_x` e `E_y`;
- a busca modal do executável já minimiza `|det(A)|` e grava esse residual nas saídas CSV, mas isso ainda não equivale à busca rigorosa dos zeros da formulação vetorial completa.
- a montagem já inclui agora, de forma separada e auditável, a parte volumétrica regular de `\varepsilon \nabla(1/\varepsilon)` multiplicando `\mathrm{grad}'\,G`;
- o termo distribucional de fronteira introduzido na Eq. (4) já aparece de forma explícita no código como soma sobre segmentos de borda da malha, com normal externa, comprimento e salto de `1/\varepsilon` registrados separadamente;
- esse termo de fronteira ainda deve ser lido como uma aproximação auditável por quadratura de linha em segmentos, não como tratamento final e fechado da formulação vetorial do artigo.

Portanto, o projeto já saiu do estágio de infraestrutura pura, mas qualquer resultado modal ainda deve ser lido como protótipo, não como reprodução validada do artigo.

## 3. Convenção geométrica do artigo versus convenção do código

A leitura atual deve ser feita assim:

- `docs/02_teoria.md` foi mantido fiel à forma do artigo original;
- `docs/02_formulacao_do_problema_de_valor_de_contorno.md` fixa a convenção operacional usada pelo código e pelos casos executáveis;
- a diferença entre as duas convenções já não é tratada como ambiguidade editorial em aberto, e sim como uma divergência explicitamente documentada entre artigo e implementação.

## 4. Ação prática derivada desta auditoria

Para alinhar código e documentação sem inventar física nova, a próxima camada de trabalho foi definida como:

- separar casos em arquivos JSON;
- organizar saídas em `out/<caso>/`;
- manter scripts reproduzíveis em `scripts/`;
- adicionar um teste de fumaça mínimo;
- preservar no código e nas saídas a marcação explícita de que o solver ainda é um protótipo.

## 5. Leitura atual após o fechamento documental da Fase 1

Da lista acima, os seguintes pontos já podem ser tratados como atendidos nesta etapa:

- os casos já estão separados em arquivos JSON reproduzíveis;
- o fluxo de saída já se organiza em `out/` por caso ou por figura;
- os scripts principais já estão centralizados em `scripts/`;
- o repositório já possui testes de fumaça e testes estruturais para Green e montagem;
- a rastreabilidade documental entre problema físico, notação, figuras e casos executáveis já está explícita em `docs/02_*`, `docs/03.3_exemplos.md` e `docs/03_resultados_numericos.md`.

O que permanece aberto a partir desta auditoria já não é mais a infraestrutura mínima da Fase 1, e sim:

- limitações conhecidas do solver protótipo, que ficam para as fases seguintes.
