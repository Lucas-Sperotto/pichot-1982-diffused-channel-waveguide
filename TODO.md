# TODO

Este arquivo consolida as pendências abertas ao final da Fase 2 e o ponto de partida da Fase 3.

O objetivo é manter, na raiz do repositório, uma visão curta do que ainda precisa ser acompanhado sem espalhar a leitura por vários documentos de auditoria.

## 2. Pendências científico-numéricas para a Fase 2 em diante

- [OK] Abrir uma trilha documental explícita entre as equações do artigo e os passos do código atual.
  Resultado desta revisão: `docs/12_trilha_equacoes_para_codigo.md` agora descreve a discretização de $\mathcal{D}_2$, as funções-base step, a montagem de `A`, a avaliação de `G = G_S + G_NS` e o critério operacional hoje usado para localizar $\beta$.
- [ ] Fechar a formulação vetorial completa do artigo no lugar do operador escalar/vetorial híbrido atual.
- [ ] Substituir a busca modal aproximada por um procedimento mais rigoroso de localização dos zeros de $\det(A)$.
- [ ] Melhorar a regularização e a quadratura da contribuição de fronteira.
- [ ] Medir convergência de malha para as figuras 2, 3, 4 e 6 antes de declarar as curvas como reproduções finais.
- [ ] Comparar quantitativamente as curvas geradas com referências externas digitizadas ou tabuladas.
- [ ] Verificar se o campo reconstruído da Figura 5 representa fielmente o modo $E^y_{21}$, e não apenas um vetor quase-nulo no $\beta$ informado pelo artigo.

## 3. Pendências operacionais de manutenção

- [ ] Manter o `README.md` e os documentos de execução alinhados com o fluxo real baseado em `scripts/build.sh`, `scripts/run_case.sh` e `scripts/generate_figures_2_to_6_csvs.sh`.
- [ ] Reavaliar periodicamente os testes de fumaça quando o contrato de saída (`results.csv`, `dispersion_curve.csv`, `field_map.csv`, `output_manifest.json`) mudar.

## 4. Leitura rápida do status da Fase 2


A abertura formal da Fase 2 agora está em `docs/12_trilha_equacoes_para_codigo.md`.
