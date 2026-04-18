import pandas as pd
import matplotlib.pyplot as plt
import os
import argparse

def plot_dispersion_curve(csv_path, output_dir):
    """
    Lê os dados do CSV e plota a curva de dispersão B vs V.
    """
    try:
        df = pd.read_csv(csv_path)
    except FileNotFoundError:
        print(f"Erro: Arquivo de dados '{csv_path}' não encontrado.")
        print("Execute primeiro 'scripts/run_case.sh <caso.json>' para gerar o CSV.")
        return

    # Exemplo de colunas esperadas no CSV: 'V_param', 'B_norm'
    # O parâmetro V é uma frequência normalizada, comum em guias de onda.
    # B é o índice efetivo normalizado.
    if 'V_param' not in df.columns or 'B_norm' not in df.columns:
        print(f"Erro: O CSV '{csv_path}' deve conter as colunas 'V_param' e 'B_norm'.")
        return

    fig, ax = plt.subplots(figsize=(8, 6))

    ax.plot(df['V_param'], df['B_norm'], 'o-', label='Este Trabalho (Eq. Integral)')
    
    # TODO: Adicionar dados de outros métodos para comparação (carregar de outros CSVs ou hardcoded)
    # Exemplo: ax.plot(V_marcatili, B_marcatili, '--', label='Marcatili (Aprox.)')

    ax.set_xlabel('Frequência Normalizada (V)')
    ax.set_ylabel('Índice Efetivo Normalizado (B)')
    ax.set_title('Curva de Dispersão do Modo Fundamental')
    ax.set_xlim(left=0)
    ax.set_ylim(0, 1)
    ax.grid(True, linestyle='--', alpha=0.6)
    ax.legend()

    output_path = os.path.join(output_dir, 'figura_dispersao.png')
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
        default='out/fig4_parabolic_1d/results.csv',
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
