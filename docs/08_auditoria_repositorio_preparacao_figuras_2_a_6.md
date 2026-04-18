# 08. Auditoria do repositório para as figuras 2 a 6

Esta auditoria complementa `docs/06_auditoria_inicial_do_repositorio.md`.

O objetivo agora não é mais perguntar apenas se o repositório está “organizado”, e sim se ele já está produzindo artefatos mais próximos do conteúdo efetivo das figuras 2 a 6.

## 1. Veredito executivo

O repositório evoluiu de “infraestrutura preparada” para “infraestrutura operacional com parâmetros calibrados e campo reconstruído”.

Em termos práticos, isso significa que:

- as figuras 2, 3, 4 e 6 já usam parâmetros lidos das próprias imagens do artigo;
- a abcissa das curvas passou a seguir a normalização mostrada nas figuras;
- a figura 5 deixou de ser apenas uma malha de amostragem e passou a gerar um mapa de campo;
- o solver agora registra um residual modal associado ao vetor quase-nulo do sistema.

O que ainda falta continua sendo maturidade numérica e validação científica fina, não organização básica.

## 2. Auditoria das entradas

### 2.1 Coleção das figuras

As figuras 2 a 6 continuam centralizadas em `data/input/figures/`.

O ganho desta etapa é que os casos deixaram de ser apenas placeholders aproximados e passaram a refletir os parâmetros que aparecem nas próprias figuras:

- figura 2: `n1 = n3 = 1.01`, `n2 = 1.05`, `a = 2b`;
- figura 3: `n1 = 1.0`, `n2 = 1.5`, `n3 = 1.43`, `a = b`;
- figura 4: `n1 = 1.0`, `n3 = 1.44`, `n2m = 1.50`, `\bar{n}_2 = 1.48`, `a = 2b`;
- figura 5: `lambda0 = 0.6328 um`, `beta/k0 = 1.4447`, `a = 2.22 um`, `a = 2b`, `n1 = 1.0`, `n3 = 1.44`, `n2m = 1.5`;
- figura 6: `n1 = 1.0`, `n3 = 1.44`, `n2m = 1.50`, `\bar{n}_2 = 1.47`, `a = 2b`.

### 2.2 Manifest

O arquivo `data/input/figures/manifest_figures_2_to_6.csv` segue sendo a referência operacional do lote.

Agora ele também registra, por status, que os casos foram calibrados a partir do próprio artigo e que a figura 5 passou para reconstrução de campo.

## 3. Auditoria das saídas

### 3.1 Curvas de dispersão

As figuras 2, 3, 4 e 6 continuam produzindo `dispersion_curve.csv`, mas com uma diferença importante:

- a coluna horizontal agora representa a abcissa normalizada do artigo;
- a saída inclui `beta_over_k0`, `normalized_beta`, `det_abs` e `modal_residual`.

Com isso, a curva deixa de ser apenas uma varredura interna do protótipo e passa a conversar de forma mais direta com os eixos publicados.

### 3.2 Mapa de campo da figura 5

O caso da figura 5 passou a gerar:

- `field_map.csv`;
- `field_sampling_grid.csv`;
- `mode_coefficients.csv`;
- `field_map_status.txt`.

O `field_map.csv` já contém valores do campo reconstruído por componente solicitado, além de `beta`, `beta_over_k0` e `modal_residual`.

## 4. Auditoria do solver

### 4.1 O que melhorou

- a busca ainda usa o operador discretizado atual, mas a solução modal não fica mais resumida ao determinante;
- o código agora estima um vetor quase-nulo do sistema por menor singularidade do operador discretizado;
- isso permite medir um residual modal e reconstruir um campo a partir de coeficientes por célula.

### 4.2 O que continua limitado

- a formulação vetorial completa do artigo ainda não está fechada;
- a busca modal ainda não é um resolvedor rigoroso dos zeros exatos de $\det(A)$;
- o termo de fronteira ainda usa uma aproximação auditável por segmentos;
- a seleção modal para distinguir modos próximos ao corte ainda merece revisão mais forte.

## 5. Auditoria dos testes e scripts

### 5.1 Scripts

`scripts/run_case.sh` e `scripts/generate_figures_2_to_6_csvs.sh` continuam sendo as interfaces principais.

A diferença é que o lote agora produz artefatos mais ricos sem mudar o contrato operacional do usuário.

### 5.2 Testes

O repositório passou a validar explicitamente:

- o novo cabeçalho das curvas com a abcissa do artigo;
- a geração do `field_map.csv`;
- o status reconstruído da figura 5;
- a existência de um vetor modal finito e de residual modal finito.

Os testes já existentes podem ser lidos assim:

- `tests/run_green_function_test.sh` compila e executa `tests/test_green_function.cpp`, verificando simetria, decaimento e derivadas do núcleo de Green;
- `tests/run_matrix_solver_test.sh` compila e executa `tests/test_matrix_solver.cpp`, verificando montagem, acoplamentos, termo de fronteira, quadratura e solução modal protótipo;
- `tests/smoke_case_loading.sh` valida o encadeamento mínimo do executável em um caso homogêneo simples;
- `tests/smoke_figures_csv_generation.sh` valida a geração dos artefatos principais das Figuras 2 a 6 e a reconstrução de campo da Figura 5.

## 6. Conclusão desta auditoria

Depois desta revisão, o repositório pode ser descrito assim:

- organizado para reprodução figura a figura;
- calibrado com parâmetros retirados das próprias figuras do artigo;
- capaz de gerar curvas com a normalização correta de eixos;
- capaz de gerar um mapa de campo para a figura 5;
- ainda dependente de validação quantitativa e de refinamento científico do solver.

Em resumo: já não estamos apenas “preparando CSVs”; agora estamos gerando artefatos numericamente interpretáveis. O que resta é reduzir a distância entre esses artefatos e a solução vetorial final do artigo.

## 7. Leitura para fechamento da Fase 1

Depois da revisão documental atual, esta auditoria pode ser interpretada assim:

- o repositório já possui uma base documental explícita para geometria, notação, figuras-alvo e casos de teste;
- os arquivos de entrada das figuras já estão calibrados e centralizados;
- os scripts principais de execução já estão definidos;
- os testes existentes já cobrem tanto o caminho operacional mínimo quanto os blocos matemáticos mais sensíveis do protótipo.

Portanto, o que ainda falta não é “descobrir o que reproduzir” nem “definir como chamar os casos”, e sim melhorar a fidelidade numérica do solver. Esse restante pertence à Fase 2 em diante, não à fundação documental da Fase 1.
