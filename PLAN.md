Fase 1 — fundação documental
Traduzir o problema físico do artigo, definir notação, variáveis, domínios, perfis de índice e exatamente quais figuras serão alvo da reprodução.

Fase 2 — formulação computacional
Transformar as equações do artigo em passos algorítmicos claros: discretização da região do guia, funções-base em degrau, montagem da matriz, avaliação do núcleo de Green e critério para encontrar β. O artigo diz explicitamente que a solução foi obtida com método dos momentos e funções-base do tipo step.

Fase 3 — primeiro caso simples
Começar pelo caso mais estável: um perfil homogêneo simples, para validar montagem de matriz e busca modal antes de atacar a difusão.

Fase 4 — casos difundidos
Implementar primeiro o perfil parabólico 1D e depois o caso circular 2D, porque eles aparecem claramente como estudos numéricos no artigo.

Fase 5 — validação visual e quantitativa
1.  **Digitalização de Referências**: Extrair dados das curvas publicadas no artigo (Goell, Yeh, etc.) usando uma ferramenta de digitalização de gráficos.
2.  **Comparação Quantitativa**: Gerar gráficos sobrepostos das curvas simuladas e das referências digitalizadas. Calcular métricas de erro (e.g., erro quadrático médio).
3.  **Verificação de Modos**: Para mapas de campo (Fig. 5), inspecionar visualmente a estrutura do campo (número de máximos e simetria) para confirmar a identificação do modo (e.g., $E^y_{21}$).

Fase 6 — empacotamento do repositório
README forte, instruções de compilação, exemplos reproduzíveis, imagens, discussão de limitações e referências.
