# 4. Conclusões

Desenvolvemos um método analítico-numérico geral e versátil, utilizando uma equação integral vetorial, para analisar o problema de propagação de ondas em um guia inomogêneo embutido em um substrato. Por meio desse método, não apenas as constantes de propagação podem ser obtidas, mas também a distribuição do campo no interior e no exterior do guia.

A precisão dos valores obtidos pode ser controlada de forma direta pelo aumento do número de pontos de amostragem, desde que os erros de arredondamento permaneçam razoáveis e limitados pela capacidade computacional da máquina. Assim, o tempo de computação é baixo para um guia que suporte os modos dominantes $E^{x}_{11}$ e $E^{y}_{11}$ (alguns segundos para o cálculo de $\beta/k_0$ e da distribuição de campo em um Amdahl 470/v7), mas pode se tornar proibitivo para guias multimodo.

Por essa razão, foi também desenvolvido um método aproximado utilizando o método do índice efetivo e a equação integral para um guia de lâmina inomogênea, reduzindo o tempo de computação e o uso de memória, ao mesmo tempo em que fornece bons resultados.

O método da equação integral vetorial pode ser estendido para guias inomogêneos de forma arbitrária embutidos em meios estratificados, mediante a modificação da função de Green, bem como para circuitos integrados de micro-ondas e submilimétricos. Ele também pode ser utilizado no estudo da difração de objetos bidimensionais sob incidência tridimensional com ângulo arbitrário.

## Agradecimentos

O autor agradece ao Prof. J. Ch. Bolomey e ao Prof. W. Tabbara pelos comentários úteis e pela leitura crítica do manuscrito.
