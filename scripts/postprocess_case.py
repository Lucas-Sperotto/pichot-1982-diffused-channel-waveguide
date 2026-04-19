#!/usr/bin/env python3

import argparse
import csv
import json
import math
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt


FIG02_REFERENCE_MANIFEST_PATH = Path("data/reference/fig_02_references_manifest.json")
FIG02_LEGACY_OVERLAY_FILENAME = "fig_02_integral_equation_overlay.png"
FIG02_VALIDATION_OVERLAY_FILENAME = "fig_02_validation_overlay.png"
FIG02_VALIDATION_MANIFEST_FILENAME = "validation_manifest.json"


def load_json(path: Path) -> dict:
    with path.open(encoding="utf-8") as handle:
        return json.load(handle)


def write_json(path: Path, payload: dict):
    with path.open("w", encoding="utf-8") as handle:
        json.dump(payload, handle, indent=2, sort_keys=True)
        handle.write("\n")


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


def load_dispersion_points(csv_path: Path, x_column=None, y_column=None):
    with csv_path.open(newline="", encoding="utf-8") as handle:
        reader = csv.DictReader(handle)
        if reader.fieldnames is None:
            raise ValueError(f"CSV vazio ou inválido: {csv_path}")

        if x_column is None or y_column is None:
            x_column, y_column = infer_dispersion_columns(reader.fieldnames)

        points = []
        for row in reader:
            points.append((float(row[x_column]), float(row[y_column])))

    points.sort(key=lambda pair: pair[0])
    return x_column, y_column, points


def interpolate_linearly(x_value, reference_points):
    if not reference_points:
        raise ValueError("A referência não contém pontos.")

    if x_value < reference_points[0][0] or x_value > reference_points[-1][0]:
        raise ValueError("Tentativa de interpolação fora do intervalo da referência.")

    for index, (x_left, y_left) in enumerate(reference_points):
        if math.isclose(x_value, x_left, rel_tol=0.0, abs_tol=1e-15):
            return y_left
        if x_value < x_left:
            x_right, y_right = reference_points[index]
            previous_x, previous_y = reference_points[index - 1]
            alpha = (x_value - previous_x) / (x_right - previous_x)
            return previous_y + alpha * (y_right - previous_y)

    return reference_points[-1][1]


def build_comparison_rows(simulation_points, reference_points):
    overlap_min = max(simulation_points[0][0], reference_points[0][0])
    overlap_max = min(simulation_points[-1][0], reference_points[-1][0])
    if overlap_min >= overlap_max:
        raise ValueError("Não há sobreposição em x entre a simulação e a referência.")

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
        raise ValueError("A comparação interpolada não produziu amostras.")

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


def compute_metrics(rows, reference_id, reference_label):
    sample_count = len(rows)
    absolute_errors = [row["absolute_error"] for row in rows]
    squared_errors = [row["squared_error"] for row in rows]
    return {
        "reference_curve": reference_id,
        "reference_label": reference_label,
        "sample_count": sample_count,
        "x_min": rows[0]["article_x_param"],
        "x_max": rows[-1]["article_x_param"],
        "mean_absolute_error": sum(absolute_errors) / sample_count,
        "max_absolute_error": max(absolute_errors),
        "root_mean_squared_error": math.sqrt(sum(squared_errors) / sample_count),
    }


def plot_reference_overlay(output_path: Path, simulation_points, references, title):
    fig, ax = plt.subplots(figsize=(8, 6))
    sim_x = [pair[0] for pair in simulation_points]
    sim_y = [pair[1] for pair in simulation_points]
    ax.plot(sim_x, sim_y, "o-", label="Este Trabalho", linewidth=1.6, markersize=4.0)

    min_y = min(sim_y)
    for reference in references:
        ref_points = reference["points"]
        ref_x = [pair[0] for pair in ref_points]
        ref_y = [pair[1] for pair in ref_points]
        min_y = min(min_y, min(ref_y))
        ax.plot(
            ref_x,
            ref_y,
            linestyle=reference.get("line_style", "--"),
            linewidth=reference.get("line_width", 1.4),
            marker=reference.get("marker", ""),
            markersize=reference.get("marker_size", 0.0),
            label=reference["label"],
        )

    ax.set_xlabel(r"$\left(\frac{2b}{\lambda_0}\right)\sqrt{n_2^2-n_1^2}$")
    ax.set_ylabel(r"$\frac{\left(\beta/k_0\right)^2-n_1^2}{n_2^2-n_1^2}$")
    ax.set_title(title)
    ax.set_xlim(left=0.0)
    ax.set_ylim(bottom=min(0.0, min_y - 0.05))
    ax.grid(True, linestyle="--", alpha=0.5)
    ax.legend()
    fig.tight_layout()
    fig.savefig(output_path, dpi=200)
    plt.close(fig)


def should_process_fig02(case_data: dict) -> bool:
    study_kind = case_data.get("study", {}).get("kind", "dispersion_curve")
    output_spec = case_data.get("output", {})
    return study_kind == "dispersion_curve" and output_spec.get("figure_id") == "fig_02"


def load_fig02_reference_manifest(repo_root: Path):
    manifest_path = repo_root / FIG02_REFERENCE_MANIFEST_PATH
    manifest = load_json(manifest_path)
    references = []

    for entry in sorted(manifest.get("references", []), key=lambda item: item["plot_order"]):
        csv_path = repo_root / entry["csv_path"]
        _, _, points = load_dispersion_points(csv_path, entry["x_column"], entry["y_column"])
        references.append(
            {
                "reference_id": entry["reference_id"],
                "label": entry["label"],
                "source_kind": entry["source_kind"],
                "csv_path": str(csv_path),
                "x_column": entry["x_column"],
                "y_column": entry["y_column"],
                "plot_order": entry["plot_order"],
                "line_style": entry.get("line_style", "--"),
                "line_width": entry.get("line_width", 1.4),
                "marker": entry.get("marker", ""),
                "marker_size": entry.get("marker_size", 0.0),
                "source_note": entry.get("source_note", ""),
                "points": points,
            }
        )

    if not references:
        raise ValueError("O manifesto de referências da Figura 2 não contém curvas.")

    return manifest_path, references


def process_fig02(case_path: Path, output_dir: Path):
    case_data = load_json(case_path)
    if not should_process_fig02(case_data):
        return

    repo_root = Path(__file__).resolve().parents[1]
    simulation_csv = output_dir / case_data.get("output", {}).get("canonical_csv_name", "results.csv")
    _, _, simulation_points = load_dispersion_points(simulation_csv)
    reference_manifest_path, references = load_fig02_reference_manifest(repo_root)

    validation_manifest = {
        "figure_id": "fig_02",
        "simulation_csv": str(simulation_csv),
        "reference_manifest_path": str(reference_manifest_path),
        "references": [],
        "artifacts": {},
    }

    for reference in references:
        comparison_rows = build_comparison_rows(simulation_points, reference["points"])
        comparison_csv = output_dir / f"fig_02_{reference['reference_id']}_comparison.csv"
        metrics_json = output_dir / f"fig_02_{reference['reference_id']}_metrics.json"
        metrics = compute_metrics(comparison_rows, reference["reference_id"], reference["label"])

        write_comparison_csv(comparison_csv, comparison_rows)
        write_json(metrics_json, metrics)

        validation_manifest["references"].append(
            {
                "reference_id": reference["reference_id"],
                "label": reference["label"],
                "source_kind": reference["source_kind"],
                "source_csv": reference["csv_path"],
                "source_note": reference["source_note"],
                "comparison_csv": str(comparison_csv),
                "metrics_json": str(metrics_json),
            }
        )

    legacy_overlay = output_dir / FIG02_LEGACY_OVERLAY_FILENAME
    integral_reference = next(
        reference for reference in references if reference["reference_id"] == "integral_equation"
    )
    plot_reference_overlay(
        legacy_overlay,
        simulation_points,
        [integral_reference],
        "Figura 2: Eq. Integral vs Curva Digitizada",
    )

    validation_overlay = output_dir / FIG02_VALIDATION_OVERLAY_FILENAME
    plot_reference_overlay(
        validation_overlay,
        simulation_points,
        references,
        "Figura 2: Validação Multi-Referência",
    )

    validation_manifest["artifacts"]["legacy_integral_overlay_png"] = str(legacy_overlay)
    validation_manifest["artifacts"]["validation_overlay_png"] = str(validation_overlay)

    validation_manifest_path = output_dir / FIG02_VALIDATION_MANIFEST_FILENAME
    write_json(validation_manifest_path, validation_manifest)

    print(f"Manifesto de validação da Figura 2 salvo em: {validation_manifest_path}")
    print(f"Overlay legado da curva integral salvo em: {legacy_overlay}")
    print(f"Overlay consolidado da Figura 2 salvo em: {validation_overlay}")


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
