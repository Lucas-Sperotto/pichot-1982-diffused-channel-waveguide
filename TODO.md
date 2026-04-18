# TODO

Este arquivo consolida as pendências abertas ao final da Fase 1.

O objetivo é manter, na raiz do repositório, uma visão curta do que ainda precisa ser acompanhado sem espalhar a leitura por vários documentos de auditoria.

## 1. Pendências editoriais e documentais da Fase 1

- [OK] Conferir diretamente no PDF do artigo a convenção de sinal de $y$ usada em `docs/02_teoria.md` e decidir se o texto traduzido será corrigido ou se a divergência ficará apenas registrada como nota editorial.
  Resultado desta revisão: `docs/02_teoria.md` foi mantido fiel ao artigo, enquanto `docs/02_formulacao_do_problema_de_valor_de_contorno.md` preserva explicitamente a convenção operacional do código.
- [OK] Conferir diretamente no PDF a escrita do perfil difundido circular 2-D da Figura 6 e revisar a documentação associada.
  Resultado desta revisão: as informações de figura e as expressões dos eixos foram consolidadas em `docs/03_resultados_numericos.md`, e `docs/09_figuras.md` passou a funcionar apenas como nota de consolidação editorial.
- [OK] Sincronizar `docs/06_auditoria_inicial_do_repositorio.md`, `docs/07_pendencias_reescritas_e_preparacao_figuras_2_a_6.md`, `docs/08_auditoria_repositorio_preparacao_figuras_2_a_6.md` e o conteúdo anteriormente duplicado em `docs/09_figuras.md`.
  Resultado desta revisão: a leitura operacional das figuras foi centralizada em `docs/03_resultados_numericos.md`, reduzindo o custo de manutenção documental.

## 2. Pendências científico-numéricas para a Fase 2 em diante

- [ ] Fechar a formulação vetorial completa do artigo no lugar do operador escalar/vetorial híbrido atual.
- [ ] Substituir a busca modal aproximada por um procedimento mais rigoroso de localização dos zeros de $\det(A)$.
- [ ] Melhorar a regularização e a quadratura da contribuição de fronteira.
- [ ] Medir convergência de malha para as figuras 2, 3, 4 e 6 antes de declarar as curvas como reproduções finais.
- [ ] Comparar quantitativamente as curvas geradas com referências externas digitizadas ou tabuladas.
- [ ] Verificar se o campo reconstruído da Figura 5 representa fielmente o modo $E^y_{21}$, e não apenas um vetor quase-nulo no $\beta$ informado pelo artigo.

## 3. Pendências operacionais de manutenção

- [ ] Manter o `README.md` e os documentos de execução alinhados com o fluxo real baseado em `scripts/build.sh`, `scripts/run_case.sh` e `scripts/generate_figures_2_to_6_csvs.sh`.
- [ ] Reavaliar periodicamente os testes de fumaça quando o contrato de saída (`results.csv`, `dispersion_curve.csv`, `field_map.csv`, `output_manifest.json`) mudar.

## 4. Leitura rápida do status da Fase 1

A Fase 1 já pode ser tratada como documentalmente estruturada porque:

- o problema físico já foi traduzido e separado da discretização;
- a notação já foi consolidada;
- os domínios e perfis já foram fixados de forma auditável;
- as figuras-alvo já foram mapeadas para casos executáveis;
- os casos de teste e seus caminhos de resolução já estão descritos em `docs/03_resultados_numericos.md`.

Os itens acima não apagam as pendências abertas; apenas deixam claro que o restante do trabalho já não é mais “fundação documental”.
