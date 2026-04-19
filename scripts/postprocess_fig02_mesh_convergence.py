#!/usr/bin/env python3

import argparse
import csv
import re
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt

from postprocess_case import build_comparison_rows
from postprocess_case import compute_metrics
from postprocess_case import load_dispersion_points
from postprocess_case import load_json
from postprocess_case import write_json


SUMMARY_FILENAME = "fig_02_mesh_convergence_summary.csv"
METRICS_FILENAME = "fig_02_mesh_convergence_metrics.json"
OVERLAY_FILENAME = "fig_02_mesh_convergence_overlay.png"


def parse_mesh_id(path: Path):
    match = re.fullmatch(r"Nx(\d+)_Ny(\d+)", path.name)
    if match is None:
        return None
    return {
        "mesh_id": path.name,
        "Nx": int(match.group(1)),
        "Ny": int(match.group(2)),
    }


def load_mesh_runs(output_dir: Path):
    mesh_runs = []
    for child in sorted(output_dir.iterdir()):
        if not child.is_dir():
            continue
        mesh_info = parse_mesh_id(child)
        if mesh_info is None:
            continue

        dispersion_csv = child / "dispersion_curve.csv"
        validation_manifest = child / "validation_manifest.json"
        if not dispersion_csv.exists() or not validation_manifest.exists():
            continue

        _, _, simulation_points = load_dispersion_points(dispersion_csv)
        manifest = load_json(validation_manifest)
        mesh_info["output_dir"] = str(child)
        mesh_info["dispersion_csv"] = str(dispersion_csv)
        mesh_info["validation_manifest"] = str(validation_manifest)
        mesh_info["simulation_points"] = simulation_points
        mesh_info["reference_entries"] = manifest["references"]
        mesh_runs.append(mesh_info)

    if not mesh_runs:
        raise ValueError("Nenhuma execução de malha da Figura 2 foi encontrada.")

    mesh_runs.sort(key=lambda item: (item["Nx"] * item["Ny"], item["Nx"], item["Ny"]))
    return mesh_runs


def write_summary_csv(output_path: Path, rows):
    with output_path.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(
            handle,
            fieldnames=[
                "mesh_id",
                "Nx",
                "Ny",
                "comparison_target_kind",
                "comparison_target_id",
                "comparison_target_label",
                "sample_count",
                "x_min",
                "x_max",
                "mean_absolute_error",
                "max_absolute_error",
                "root_mean_squared_error",
            ],
            lineterminator="\n",
        )
        writer.writeheader()
        writer.writerows(rows)


def plot_mesh_overlay(output_path: Path, mesh_runs):
    fig, ax = plt.subplots(figsize=(8, 6))
    min_y = None

    for mesh in mesh_runs:
        x_values = [point[0] for point in mesh["simulation_points"]]
        y_values = [point[1] for point in mesh["simulation_points"]]
        min_y = min(y_values) if min_y is None else min(min_y, min(y_values))
        ax.plot(x_values, y_values, marker="o", linewidth=1.4, markersize=3.5, label=mesh["mesh_id"])

    ax.set_xlabel(r"$\left(\frac{2b}{\lambda_0}\right)\sqrt{n_2^2-n_1^2}$")
    ax.set_ylabel(r"$\frac{\left(\beta/k_0\right)^2-n_1^2}{n_2^2-n_1^2}$")
    ax.set_title("Figura 2: Convergência de Malha")
    ax.set_xlim(left=0.0)
    ax.set_ylim(bottom=min(0.0, (min_y if min_y is not None else 0.0) - 0.05))
    ax.grid(True, linestyle="--", alpha=0.5)
    ax.legend(title="Malhas")
    fig.tight_layout()
    fig.savefig(output_path, dpi=200)
    plt.close(fig)


def build_convergence_summary(output_dir: Path):
    mesh_runs = load_mesh_runs(output_dir)
    baseline_mesh = mesh_runs[-1]
    summary_rows = []

    for mesh in mesh_runs:
        for reference_entry in mesh["reference_entries"]:
            metrics = load_json(Path(reference_entry["metrics_json"]))
            summary_rows.append(
                {
                    "mesh_id": mesh["mesh_id"],
                    "Nx": mesh["Nx"],
                    "Ny": mesh["Ny"],
                    "comparison_target_kind": "reference",
                    "comparison_target_id": reference_entry["reference_id"],
                    "comparison_target_label": reference_entry["label"],
                    "sample_count": metrics["sample_count"],
                    "x_min": metrics["x_min"],
                    "x_max": metrics["x_max"],
                    "mean_absolute_error": metrics["mean_absolute_error"],
                    "max_absolute_error": metrics["max_absolute_error"],
                    "root_mean_squared_error": metrics["root_mean_squared_error"],
                }
            )

        baseline_rows = build_comparison_rows(
            mesh["simulation_points"], baseline_mesh["simulation_points"]
        )
        baseline_metrics = compute_metrics(
            baseline_rows,
            baseline_mesh["mesh_id"],
            f"Baseline {baseline_mesh['mesh_id']}",
        )
        summary_rows.append(
            {
                "mesh_id": mesh["mesh_id"],
                "Nx": mesh["Nx"],
                "Ny": mesh["Ny"],
                "comparison_target_kind": "baseline_mesh",
                "comparison_target_id": baseline_mesh["mesh_id"],
                "comparison_target_label": f"Baseline {baseline_mesh['mesh_id']}",
                "sample_count": baseline_metrics["sample_count"],
                "x_min": baseline_metrics["x_min"],
                "x_max": baseline_metrics["x_max"],
                "mean_absolute_error": baseline_metrics["mean_absolute_error"],
                "max_absolute_error": baseline_metrics["max_absolute_error"],
                "root_mean_squared_error": baseline_metrics["root_mean_squared_error"],
            }
        )

    summary_csv = output_dir / SUMMARY_FILENAME
    write_summary_csv(summary_csv, summary_rows)

    overlay_png = output_dir / OVERLAY_FILENAME
    plot_mesh_overlay(overlay_png, mesh_runs)

    metrics_json = output_dir / METRICS_FILENAME
    write_json(
        metrics_json,
        {
            "figure_id": "fig_02",
            "baseline_mesh_id": baseline_mesh["mesh_id"],
            "baseline_mesh": {
                "Nx": baseline_mesh["Nx"],
                "Ny": baseline_mesh["Ny"],
            },
            "mesh_count": len(mesh_runs),
            "mesh_ids": [mesh["mesh_id"] for mesh in mesh_runs],
            "summary_csv": str(summary_csv),
            "overlay_png": str(overlay_png),
        },
    )


def main():
    parser = argparse.ArgumentParser(
        description="Agrega as execuções de convergência de malha da Figura 2."
    )
    parser.add_argument("--output-dir", required=True, help="Diretório raiz da convergência.")
    args = parser.parse_args()

    build_convergence_summary(Path(args.output_dir).resolve())


if __name__ == "__main__":
    main()
