#!/usr/bin/env python3

import argparse
import csv
import json
import math
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt


FIG02_REFERENCE_PATH = Path("data/reference/fig_02_integral_equation_digitized.csv")
FIG02_COMPARISON_FILENAME = "fig_02_integral_equation_comparison.csv"
FIG02_METRICS_FILENAME = "fig_02_integral_equation_metrics.json"
FIG02_OVERLAY_FILENAME = "fig_02_integral_equation_overlay.png"


def load_json(path: Path) -> dict:
    with path.open(encoding="utf-8") as handle:
        return json.load(handle)


def infer_dispersion_columns(fieldnames):
    fieldname_set = set(fieldnames)
    if {"article_x_param", "normalized_beta"}.issubset(fieldname_set):
        return "article_x_param", "normalized_beta"
    if {"V_param", "B_norm"}.issubset(fieldname_set):
        return "V_param", "B_norm"
    raise ValueError(
        "O CSV de dispersão precisa conter ('article_x_param', 'normalized_beta') "
        "ou ('V_param', 'B_norm')."
    )


def load_dispersion_points(csv_path: Path):
    with csv_path.open(newline="", encoding="utf-8") as handle:
        reader = csv.DictReader(handle)
        if reader.fieldnames is None:
            raise ValueError(f"CSV vazio ou inválido: {csv_path}")
        x_column, y_column = infer_dispersion_columns(reader.fieldnames)
        points = []
        for row in reader:
            points.append((float(row[x_column]), float(row[y_column])))

    points.sort(key=lambda pair: pair[0])
    return x_column, y_column, points


def interpolate_linearly(x_value, reference_points):
    if not reference_points:
        raise ValueError("A referência digitizada não contém pontos.")

    if x_value < reference_points[0][0] or x_value > reference_points[-1][0]:
        raise ValueError("Tentativa de interpolação fora do intervalo da referência digitizada.")

    for index, (x_left, y_left) in enumerate(reference_points):
        if math.isclose(x_value, x_left, rel_tol=0.0, abs_tol=1e-15):
            return y_left
        if x_value < x_left:
            x_right, y_right = reference_points[index]
            previous_x, previous_y = reference_points[index - 1]
            alpha = (x_value - previous_x) / (x_right - previous_x)
            return previous_y + alpha * (y_right - previous_y)

    return reference_points[-1][1]


def build_fig02_comparison(simulation_points, reference_points):
    overlap_min = max(simulation_points[0][0], reference_points[0][0])
    overlap_max = min(simulation_points[-1][0], reference_points[-1][0])
    if overlap_min >= overlap_max:
        raise ValueError("Não há sobreposição em x entre a simulação e a referência digitizada.")

    comparison_rows = []
    for article_x_param, simulated_beta in simulation_points:
        if article_x_param < overlap_min or article_x_param > overlap_max:
            continue

        reference_beta = interpolate_linearly(article_x_param, reference_points)
        abs_error = abs(simulated_beta - reference_beta)
        comparison_rows.append(
            {
                "article_x_param": article_x_param,
                "normalized_beta_simulation": simulated_beta,
                "normalized_beta_reference": reference_beta,
                "absolute_error": abs_error,
                "squared_error": abs_error * abs_error,
            }
        )

    if not comparison_rows:
        raise ValueError("A comparação interpolada da Figura 2 não produziu amostras.")

    return comparison_rows


def write_comparison_csv(output_path: Path, rows):
    with output_path.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(
            handle,
            fieldnames=[
                "article_x_param",
                "normalized_beta_simulation",
                "normalized_beta_reference",
                "absolute_error",
                "squared_error",
            ],
            lineterminator="\n",
        )
        writer.writeheader()
        writer.writerows(rows)


def compute_metrics(rows):
    sample_count = len(rows)
    absolute_errors = [row["absolute_error"] for row in rows]
    squared_errors = [row["squared_error"] for row in rows]
    return {
        "reference_curve": "fig_02_integral_equation_digitized",
        "sample_count": sample_count,
        "x_min": rows[0]["article_x_param"],
        "x_max": rows[-1]["article_x_param"],
        "mean_absolute_error": sum(absolute_errors) / sample_count,
        "max_absolute_error": max(absolute_errors),
        "root_mean_squared_error": math.sqrt(sum(squared_errors) / sample_count),
    }


def write_metrics_json(output_path: Path, metrics: dict):
    with output_path.open("w", encoding="utf-8") as handle:
        json.dump(metrics, handle, indent=2, sort_keys=True)
        handle.write("\n")


def plot_overlay(output_path: Path, simulation_points, reference_points):
    fig, ax = plt.subplots(figsize=(8, 6))
    sim_x = [pair[0] for pair in simulation_points]
    sim_y = [pair[1] for pair in simulation_points]
    ref_x = [pair[0] for pair in reference_points]
    ref_y = [pair[1] for pair in reference_points]

    ax.plot(sim_x, sim_y, "o-", label="Este Trabalho", linewidth=1.6, markersize=4.0)
    ax.plot(ref_x, ref_y, "--", label="Artigo Digitizado", linewidth=1.4)
    ax.set_xlabel(r"$\left(\frac{2b}{\lambda_0}\right)\sqrt{n_2^2-n_1^2}$")
    ax.set_ylabel(r"$\frac{\left(\beta/k_0\right)^2-n_1^2}{n_2^2-n_1^2}$")
    ax.set_title("Figura 2: Eq. Integral vs Curva Digitizada")
    ax.set_xlim(left=0.0)
    ax.set_ylim(bottom=0.0)
    ax.grid(True, linestyle="--", alpha=0.5)
    ax.legend()
    fig.tight_layout()
    fig.savefig(output_path, dpi=200)
    plt.close(fig)


def should_process_fig02(case_data: dict) -> bool:
    study_kind = case_data.get("study", {}).get("kind", "dispersion_curve")
    output_spec = case_data.get("output", {})
    return study_kind == "dispersion_curve" and output_spec.get("figure_id") == "fig_02"


def process_fig02(case_path: Path, output_dir: Path):
    case_data = load_json(case_path)
    if not should_process_fig02(case_data):
        return

    repo_root = Path(__file__).resolve().parents[1]
    simulation_csv = output_dir / case_data.get("output", {}).get("canonical_csv_name", "results.csv")
    reference_csv = repo_root / FIG02_REFERENCE_PATH
    _, _, simulation_points = load_dispersion_points(simulation_csv)
    _, _, reference_points = load_dispersion_points(reference_csv)

    comparison_rows = build_fig02_comparison(simulation_points, reference_points)
    comparison_csv = output_dir / FIG02_COMPARISON_FILENAME
    metrics_json = output_dir / FIG02_METRICS_FILENAME
    overlay_png = output_dir / FIG02_OVERLAY_FILENAME

    write_comparison_csv(comparison_csv, comparison_rows)
    write_metrics_json(metrics_json, compute_metrics(comparison_rows))
    plot_overlay(overlay_png, simulation_points, reference_points)

    print(f"Comparação da Figura 2 salva em: {comparison_csv}")
    print(f"Métricas da Figura 2 salvas em: {metrics_json}")
    print(f"Overlay da Figura 2 salvo em: {overlay_png}")


def main():
    parser = argparse.ArgumentParser(description="Executa pós-processamentos reproduzíveis por caso.")
    parser.add_argument("--case-file", required=True, help="Arquivo JSON do caso executado.")
    parser.add_argument("--output-dir", required=True, help="Diretório de saída já gerado pelo solver.")
    args = parser.parse_args()

    case_path = Path(args.case_file).resolve()
    output_dir = Path(args.output_dir).resolve()
    process_fig02(case_path, output_dir)


if __name__ == "__main__":
    main()
