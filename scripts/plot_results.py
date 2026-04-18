import pandas as pd
import matplotlib.pyplot as plt
import os
import argparse

def infer_dispersion_columns(df):
    if {'article_x_param', 'normalized_beta'}.issubset(df.columns):
        return 'article_x_param', 'normalized_beta', 'Abscissa do Artigo', 'Beta Normalizado'
    if {'V_param', 'B_norm'}.issubset(df.columns):
        return 'V_param', 'B_norm', 'Frequência Normalizada (V)', 'Índice Efetivo Normalizado (B)'
    raise ValueError(
        "O CSV precisa conter as colunas ('article_x_param', 'normalized_beta') "
        "ou, por compatibilidade legada, ('V_param', 'B_norm')."
    )

def plot_dispersion_curve(csv_path, output_dir):
    """
    Lê os dados do CSV e plota a curva de dispersão no formato atual do repositório.
    """
    try:
        df = pd.read_csv(csv_path)
    except FileNotFoundError:
        print(f"Erro: Arquivo de dados '{csv_path}' não encontrado.")
        print("Execute primeiro 'scripts/run_case.sh <caso.json>' para gerar o CSV.")
        return

    try:
        x_col, y_col, x_label, y_label = infer_dispersion_columns(df)
    except ValueError as exc:
        print(f"Erro: {exc}")
        return

    fig, ax = plt.subplots(figsize=(8, 6))

    ax.plot(df[x_col], df[y_col], 'o-', label='Este Trabalho (Eq. Integral)')
    
    # TODO: Adicionar dados de outros métodos para comparação (carregar de outros CSVs ou hardcoded)
    # Exemplo: ax.plot(V_marcatili, B_marcatili, '--', label='Marcatili (Aprox.)')

    ax.set_xlabel(x_label)
    ax.set_ylabel(y_label)
    ax.set_title('Curva de Dispersão')
    ax.set_xlim(left=0)
    ax.grid(True, linestyle='--', alpha=0.6)
    ax.legend()

    output_path = os.path.join(output_dir, 'dispersion_curve.png')
    plt.savefig(output_path)
    print(f"Gráfico de dispersão salvo em: {output_path}")
    plt.close(fig)


def main():
    """
    Função principal para orquestrar a geração de gráficos.
    """
    parser = argparse.ArgumentParser(description="Gera gráficos a partir dos resultados da simulação.")
    parser.add_argument(
        '--data-file', 
        type=str, 
        default='out/figures/fig_04/fig_04_curve_A_diffused_1d_eq_integral/dispersion_curve.csv',
        help='Caminho para o arquivo de dados CSV.'
    )
    parser.add_argument(
        '--output-dir', 
        type=str, 
        default='',
        help='Diretório para salvar os gráficos gerados. Por padrão usa <diretorio_do_csv>/plots.'
    )
    args = parser.parse_args()

    output_dir = args.output_dir or os.path.join(os.path.dirname(args.data_file), 'plots')
    os.makedirs(output_dir, exist_ok=True)
    plot_dispersion_curve(args.data_file, output_dir)

if __name__ == '__main__':
    main()
