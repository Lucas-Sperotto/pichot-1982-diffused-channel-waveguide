# AGENTS.md

Este arquivo orienta agentes automatizados e fluxos assistidos por IA neste repositório.

Ele complementa, e nao substitui, o `README.md` e o `CONTRIBUTING.md`.
Para contribuidores humanos, o guia principal de colaboracao deve ser o `CONTRIBUTING.md`.
Para agentes, este arquivo preserva restricoes, prioridades e criterios de rastreabilidade que nao devem ser inferidos automaticamente.

---

## Objetivo do repositorio

Este repositorio tem como objetivo reproduzir, com clareza, fidelidade e rastreabilidade, os resultados do artigo de **Ch. Pichot**, sobre a solucao numerica exata do **diffused channel waveguide**.

A prioridade do projeto e:

- fidelidade matematica ao artigo;
- reprodutibilidade computacional;
- validacao progressiva por casos de teste;
- organizacao limpa entre documentacao, codigo, scripts e saidas;
- separacao explicita entre fisica, matematica, discretizacao, I/O e executaveis.

Este e um projeto de **reproducao numerica**, nao de improvisacao teorica.

---

## Principios gerais de desenvolvimento

- Priorizar clareza, fidelidade ao artigo e didatica.
- Nao inventar formulas ausentes.
- Quando faltar informacao, registrar a lacuna e pedir verificacao.
- Marcar ambiguidades de OCR, traducao ou notacao com `TODO`.
- Preservar ambiguidades historicas importantes quando existirem.
- Nao “corrigir” o artigo por palpite.
- Nao substituir formulacoes do artigo por variantes mais convenientes sem registro tecnico explicito.
- Nao mascarar dificuldades numericas com explicacoes vagas.
- Em caso de incerteza: **registrar, isolar e tornar auditavel**.

---

## Fontes canonicas do projeto

A ordem de referencia deve ser:

1. artigo original;
2. `docs/` do repositorio;
3. `README.md`;
4. `CONTRIBUTING.md`;
5. codigo existente e testes.

Se houver conflito entre implementacao e documentacao tecnica consolidada, o agente deve:

- nao assumir que o codigo esta correto;
- nao sobrescrever a documentacao por simplificacao;
- registrar a divergencia;
- propor ajuste rastreavel.

---

## Papel da pasta docs

A pasta `docs/` e a base documental oficial do projeto.

Ela deve conter, progressivamente:

- traducao tecnica revisada do artigo;
- interpretacao matematica adotada;
- observacoes editoriais sobre OCR e notacao;
- resumo dos casos de teste;
- rastreabilidade entre equacoes, figuras, tabelas e modulos do codigo;
- notas de implementacao e validacao.

### Regra principal para `docs/`

- nao alterar `docs/` livremente;
- nao apagar conteudo de `docs/`;
- nao reescrever trechos de `docs/` apenas para “simplificar”;
- so propor mudancas quando houver justificativa tecnica clara;
- quando houver ambiguidade, preferir **acrescentar observacao editorial** em vez de substituir o texto;
- manter rastreabilidade entre texto, equacoes, figuras e resultados numericos.

---

## Convencoes para documentacao

- Usar `$...$` para matematica inline em arquivos `.md`.
- Usar `$$...$$` para blocos matematicos em arquivos `.md`.
- Preservar a ligacao entre:
  - formulacao fisica;
  - equacoes;
  - discretizacao;
  - implementacao;
  - resultados;
  - figuras reproduzidas.
- Quando houver duvida de traducao, OCR ou notacao, explicitar a incerteza em vez de oculta-la.
- Sempre que possivel, registrar quais equacoes, figuras ou tabelas do artigo cada modulo implementa.

---

## Estrutura esperada do repositorio

O repositorio deve manter, sempre que possivel, a seguinte separacao logica:

- `docs/` → documentacao tecnica, traducao e notas editoriais
- `include/` → headers e interfaces reutilizaveis
- `src/` → implementacao em C++
- `src/apps/` → executaveis e pontos de entrada
- `scripts/` → automacao com `.sh` e scripts Python
- `cases/` ou `data/input/` → arquivos de entrada dos casos
- `tests/` → testes unitarios e testes de sanidade
- `out/` ou `data/output/` → saidas geradas
- `data/` → insumos auxiliares, tabelas e referencias numericas
- `meshes/` → apenas se houver malhas explicitamente necessarias

Nao criar estruturas paralelas desnecessarias.

---

## Regras de implementacao

- Trabalhar sempre em etapas pequenas, verificaveis e reversiveis.
- Nao tentar implementar todos os casos do artigo de uma vez.
- Antes de adicionar complexidade fisica, consolidar a infraestrutura numerica minima.
- Toda alteracao em `src/` deve preservar compilacao, ou deixar explicitado o bloqueio real.
- Toda funcionalidade nova deve ter pelo menos um teste de sanidade, validacao simples ou script demonstravel.
- Priorizar legibilidade, rastreabilidade e consistencia antes de otimizacoes prematuras.
- Nao esconder logica importante em macros obscuras.
- Separar claramente:
  - geometria e discretizacao;
  - materiais e perfis de indice;
  - montagem;
  - solver;
  - I/O;
  - pos-processamento.

---

## Estrategia de desenvolvimento

A implementacao deve seguir, em geral, esta ordem:

1. infraestrutura minima do repositorio;
2. contrato de entrada e saida;
3. leitura de casos e validacao dos parametros;
4. implementacao dos objetos fisicos e perfis de indice;
5. implementacao da formulacao numerica adotada;
6. montagem do problema linear associado;
7. busca dos autovalores / constantes de propagacao;
8. reconstrucao dos campos;
9. salvamento de resultados em formatos reprodutiveis;
10. reproducao progressiva dos casos e figuras do artigo;
11. extensao para aproximacoes auxiliares, se documentadas.

Nao pular etapas sem necessidade.

---

## Escopo cientifico minimo esperado

O agente deve reconhecer que este projeto busca reproduzir, progressivamente, pelo menos os seguintes blocos:

- formulacao do guia de canal difundido;
- solucao numerica exata adotada no artigo;
- comparacoes com metodos ja citados no texto;
- reproducao dos casos numericos principais;
- interpretacao da distribuicao de campo;
- eventual implementacao separada da aproximacao por effective index method, apenas quando devidamente documentada.

Se um trecho do artigo nao estiver suficientemente claro para implementacao confiavel:

- nao inventar;
- nao preencher lacunas por intuicao;
- registrar a pendencia em `docs/` ou `TODO`.

---

## Convencoes de entrada

Os casos devem ser configurados por arquivos em `cases/` ou `data/input/`, preferencialmente em **YAML** ou **JSON**.

Cada caso deve conter, de forma clara:

- identificacao do caso;
- parametros geometricos;
- parametros materiais;
- parametros numericos;
- configuracao do solver;
- numero de modos desejados, quando aplicavel;
- observacoes relevantes;
- referencia ao trecho do artigo que o caso pretende reproduzir.

Evitar casos fixos no codigo.

Cada executavel deve aceitar um arquivo de entrada.

---

## Convencoes de execucao

A execucao deve ser feita, sempre que possivel, por scripts em `scripts/`.

Fluxo esperado:

1. compilar;
2. executar o caso;
3. salvar resultados em pasta dedicada;
4. chamar scripts auxiliares de pos-processamento, quando necessario;
5. gerar artefatos reprodutiveis.

Evitar execucoes manuais fragmentadas quando houver script correspondente.

---

## Convencoes de saida

Toda execucao deve salvar resultados em `out/` ou `data/output/`, em subpastas por caso.

Exemplo:

`out/case01_diffused_channel/`

Cada pasta de saida deve buscar conter:

- copia ou snapshot da entrada;
- log de execucao;
- parametros resolvidos;
- autovalores ou constantes de propagacao;
- indices efetivos, quando aplicavel;
- campos reconstruidos;
- metadados numericos;
- tabelas em CSV ou JSON;
- figuras geradas por scripts externos, quando aplicavel.

Nao salvar resultados soltos na raiz do repositorio.

Toda saida numerica deve ir para **CSV** ou **JSON**.

Graficos devem ser gerados por **scripts Python externos** em `scripts/`.

---

## Regras para scripts

- scripts `.sh` sao a interface principal de build e execucao;
- scripts Python devem ser usados para apoio, pos-processamento e graficos;
- evitar duplicacao de logica entre scripts;
- preferir pequenos utilitarios reutilizaveis;
- nao mover para scripts a logica cientifica principal que deve permanecer auditavel no C++.

---

## Regras para testes

Os testes devem validar progressivamente:

- leitura de entrada;
- integridade dos parametros;
- consistencia de dimensoes;
- comportamento de funcoes basicas;
- sanidade da montagem;
- sanidade do solver;
- reproducao minima de casos conhecidos;
- estabilidade de I/O.

Quando nao houver teste unitario formal, ao menos fornecer:

- script reproduzivel;
- caso minimo;
- criterio verificavel de sucesso ou falha.

---

## Regras de estilo para o codigo C++

- Manter o nucleo numerico em **C++17**.
- Nao introduzir dependencias desnecessarias.
- Preferir clareza a abstracoes excessivas.
- Usar nomes consistentes com a formulacao do artigo.
- Documentar blocos matematicamente sensiveis.
- Comentar o codigo de forma clara e suficiente para facilitar revisao cientifica.
- Separar bem fisica, matematica, discretizacao, solver, I/O e executaveis.
- Evitar acoplamento desnecessario.
- Nao esconder detalhes importantes do metodo em funcoes opacas sem documentacao.

---

## Regras para modificacoes em arquivos existentes

Antes de editar:

- ler os arquivos relacionados;
- entender a funcao atual de cada modulo;
- verificar a compatibilidade com `docs/`;
- preservar a estrutura do projeto.

Ao final de cada tarefa, o agente deve sempre:

- resumir o que foi feito;
- listar os arquivos alterados;
- explicar como compilar;
- explicar como executar;
- apontar limitacoes restantes;
- registrar pendencias ou ambiguidades remanescentes.

---

## Regras de seguranca intelectual do projeto

Este projeto e de reproducao numerica, nao de improvisacao.

Portanto:

- nao corrigir o artigo por palpite;
- nao substituir formulacoes por variantes mais convenientes sem registro;
- nao apagar ambiguidades historicas importantes;
- nao apresentar como “exato” algo que seja apenas aproximado;
- nao esconder lacunas documentais.

Sempre que houver incerteza:

- registrar;
- isolar;
- tornar auditavel.

---

## Definicao de sucesso

Uma etapa so e considerada concluida quando:

- esta coerente com `docs/`;
- compila ou falha por motivo claramente identificado;
- possui execucao verificavel;
- gera artefato reproduzivel;
- melhora a organizacao do repositorio;
- aumenta, e nao reduz, a rastreabilidade cientifica.

Se uma alteracao deixa o repositorio mais confuso, ela nao deve ser considerada concluida.
