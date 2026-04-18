# 08. Auditoria do repositório para preparação das figuras 2 a 6

Esta auditoria complementa `docs/06_auditoria_inicial_do_repositorio.md`.

O objetivo aqui é responder a uma pergunta prática: o repositório já está organizado para gerar, de forma reproduzível, os CSVs-base das figuras 2 a 6?

## 1. Conclusão executiva

Sim, do ponto de vista de organização de entradas, saídas e automação, o repositório já está preparado para essa etapa.

O que foi consolidado:

- coleção dedicada de casos em `data/input/figures/`;
- manifest explícito em `data/input/figures/manifest_figures_2_to_6.csv`;
- diretórios de saída por figura e por caso em `out/figures/<figure_id>/<case_id>/`;
- separação entre CSV canônico do estudo e `results.csv` de compatibilidade;
- automação em lote por `scripts/generate_figures_2_to_6_csvs.sh`;
- cobertura de sanidade do pipeline por `tests/smoke_figures_csv_generation.sh`.

Portanto, a preparação de I/O deixou de ser o gargalo principal.

## 2. Auditoria das entradas

### 2.1 Coleção de casos

As figuras 2 a 6 agora são representadas por arquivos JSON específicos em `data/input/figures/`.

Cada caso carrega:

- identificação do caso;
- referência ao trecho do artigo;
- tipo de estudo;
- parâmetros geométricos e materiais;
- discretização;
- configuração do solver;
- metadados de saída;
- notas de maturidade.

### 2.2 Manifest

O arquivo `data/input/figures/manifest_figures_2_to_6.csv` passou a funcionar como índice operacional da coleção.

Ele registra, por linha:

- `figure_id`;
- `case_id`;
- `case_file`;
- `study_kind`;
- `curve_id`;
- `canonical_csv_name`;
- `status`.

Isso elimina a duplicação frágil de listas de casos dentro do script em lote e torna a coleção auditável.

## 3. Auditoria das saídas

### 3.1 Regra de organização

Para os casos da família `figures`, a convenção de saída é:

`out/figures/<figure_id>/<case_id>/`

Dentro de cada diretório, a execução passa a deixar:

- `input_snapshot.json`;
- `run_summary.txt`;
- `output_manifest.json`;
- `profile_samples.csv`;
- o CSV canônico do estudo;
- `results.csv` como compatibilidade legada.

No caso da figura 5, também aparece `field_map_status.txt`.

### 3.2 Manifest de saída

O `output_manifest.json` agora registra:

- identificação do caso;
- tipo de estudo;
- família de saída;
- `figure_id`;
- `curve_id`;
- nome do CSV canônico;
- caminhos resolvidos dos arquivos principais;
- status do artefato gerado.

Esse arquivo melhora a rastreabilidade entre entrada, execução e pós-processamento.

## 4. Auditoria dos scripts

### 4.1 Execução isolada

`scripts/run_case.sh` continua sendo a interface para um caso individual.

Quando o caso pertence à família `figures`, o script agora resolve automaticamente o diretório padrão de saída para a árvore `out/figures/...`, sem depender de um caminho manual.

### 4.2 Execução em lote

`scripts/generate_figures_2_to_6_csvs.sh` agora consome diretamente o manifest das figuras 2 a 6.

Com isso, o script:

- deixa de repetir uma lista hard-coded de casos;
- falha cedo se um arquivo listado no manifest não existir;
- gera um índice de execução `figures_2_to_6_index.csv` com o status da geração.

## 5. Achados desta auditoria

### 5.1 Pontos fortes

- a coleção das figuras 2 a 6 já está separada do restante dos casos protótipos;
- a organização de saída está coerente com o objetivo de reprodução figura a figura;
- já é possível gerar CSVs canônicos em lote para as figuras 2, 3, 4 e 6;
- a figura 5 já possui um artefato reproduzível de preparação de grade.

### 5.2 Limites ainda presentes

- o solver segue prototípico e ainda não fecha a formulação vetorial completa do artigo;
- a figura 5 ainda não produz campo físico reconstruído;
- as figuras 2, 3, 4 e 6 ainda carecem de calibração/validação quantitativa;
- a sensibilidade à malha ainda é um risco técnico real.

## 6. Veredito

Depois desta revisão, a situação do repositório é a seguinte:

- preparado para gerar os CSVs-base das figuras 2 a 6;
- preparado para organizar as saídas por figura e por curva;
- ainda não pronto para declarar reprodução científica validada dessas figuras.

Em outras palavras, a infraestrutura está pronta; a maturidade numérica ainda não.
