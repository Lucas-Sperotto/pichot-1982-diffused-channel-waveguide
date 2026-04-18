# 06. Auditoria inicial do repositório

Este documento registra a validação inicial de `docs/` e das principais divergências entre a documentação, a estrutura real do repositório e o estado atual do código.

O objetivo aqui não é reescrever a base documental existente, mas tornar explícito o que já está consistente, o que ainda é apenas estruturalmente desejado e quais pendências precisam permanecer auditáveis.

## 1. O que já está consistente em `docs/`

- `docs/00_titulo_autoria_resumo.md` identifica corretamente o artigo, autoria, periódico e escopo geral.
- `docs/01_introducao.md` resume o posicionamento histórico do método e dos trabalhos comparados.
- `docs/02_teoria.md` registra a formulação de Helmholtz, a equação integral vetorial, a função de Green e o papel do método dos momentos.
- `docs/03_resultados_numericos.md` separa os principais blocos de validação numérica por família de caso.
- `docs/04_conclusoes.md` e `docs/05_referencias.md` preservam o fechamento científico do artigo.

Em outras palavras, `docs/` já cobre bem o arco narrativo do artigo, mas ainda não cobre totalmente a rastreabilidade para a implementação.

## 2. Divergências encontradas

### 2.1 Estrutura documentada versus estrutura real

O arquivo `ESTRUTURA.md` descreve uma árvore futura com `src/`, `include/`, `cases/`, `tests/` e `results/`, mas o estado atual do repositório ainda concentra o código-fonte principal na raiz.

Isso não invalida `ESTRUTURA.md`, mas significa que ele deve ser interpretado como estrutura-alvo, não como fotografia do estado atual.

### 2.2 Build script fora de sincronia

O `build_and_run.sh` original apontava para arquivos em `src/` e `include/` que ainda não existiam no repositório. Isso criava uma divergência direta entre a interface de execução e o conteúdo realmente disponível.

### 2.3 Casos ainda não separados em entradas reproduzíveis

Até esta auditoria, o caso parabólico 1-D estava embutido diretamente em `main.cpp`, o que contrariava a diretriz do projeto de usar arquivos de entrada reproduzíveis.

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
- o termo distribucional de fronteira introduzido na Eq. (4) continua explicitamente pendente e não foi absorvido por aproximação silenciosa.

Portanto, o projeto já saiu do estágio de infraestrutura pura, mas qualquer resultado modal ainda deve ser lido como protótipo, não como reprodução validada do artigo.

## 3. Pendência editorial detectada em `docs/02_teoria.md`

Há um ponto que precisa de conferência no texto da teoria:

- o documento define `\mathcal{D}_1` como superstrato para `y > 0` e `\mathcal{D}_3` como substrato;
- mais adiante aparece a frase “No meio III (`y > 0`)”, o que conflita com a definição anterior.

`TODO`: conferir diretamente o PDF do artigo e decidir se isso é apenas um deslize editorial de sinal em `y` ou se a convenção adotada no texto precisa ser reexplicada com mais cuidado.

## 4. Ação prática derivada desta auditoria

Para alinhar código e documentação sem inventar física nova, a próxima camada de trabalho foi definida como:

- separar casos em arquivos JSON;
- organizar saídas em `out/<caso>/`;
- manter scripts reproduzíveis em `scripts/`;
- adicionar um teste de fumaça mínimo;
- preservar no código e nas saídas a marcação explícita de que o solver ainda é um protótipo.
