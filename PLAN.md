Fase 0 — auditoria inicial do repositório
Validar `docs/`, explicitar divergências entre documentação, código e estrutura real do projeto, e registrar pendências editoriais sem sobrescrever o conteúdo técnico já consolidado.

Fase 1 — infraestrutura reproduzível mínima
Separar casos em arquivos JSON, remover parâmetros fixos do executável, organizar a saída em `out/<caso>/`, manter scripts de build/execução em `scripts/` e adicionar ao menos um teste de fumaça reproduzível.

Fase 2 — casos iniciais separados
Trabalhar com três famílias de caso desde o início:
- caso de sanidade homogêneo (`smoke_homogeneous`);
- primeiro caso ligado ao artigo com difusão parabólica 1-D (`fig4_parabolic_1d`);
- caso de sanidade com perfil circular 2-D (`smoke_circular_2d`).

Fase 3 — núcleo numérico do artigo
Implementar, com rastreabilidade:
- núcleo de Green singular e não singular;
- gradientes do núcleo;
- montagem dos blocos da matriz do método dos momentos;
- cálculo consistente de `det(A)`;
- busca de `beta` por anulação de `det(A)`.

Fase 4 — validação progressiva
Usar primeiro o caso homogêneo para validar malha, montagem e busca modal; depois atacar os casos difundidos do artigo, sempre salvando CSV/JSON e logs por caso.

Fase 5 — reprodução das figuras
Comparar curvas e campos com as figuras do artigo, sobrepor resultados em scripts externos e registrar diferenças quantitativas e lacunas remanescentes.

Fase 6 — fechamento do pacote
Consolidar README, instruções de compilação, trilha de execução dos casos, limites do protótipo e vínculo entre equações, figuras, entradas e módulos do código.
