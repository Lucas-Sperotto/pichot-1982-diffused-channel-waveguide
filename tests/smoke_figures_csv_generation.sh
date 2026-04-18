#!/bin/bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
OUTPUT_DIR="$(mktemp -d /tmp/pichot_figures_csvs.XXXXXX)"
TEMP_CASE_DIR="$(mktemp -d /tmp/pichot_figures_cases.XXXXXX)"
TEMP_MANIFEST="${TEMP_CASE_DIR}/manifest_figures_2_to_6.csv"
DEFAULT_FIG05_DIR="${ROOT_DIR}/out/figures/fig_05/fig_05_field_map_preparation"

python3 - <<'PY' "${ROOT_DIR}" "${TEMP_CASE_DIR}" "${TEMP_MANIFEST}"
import csv
import json
import sys
from pathlib import Path

root_dir = Path(sys.argv[1])
temp_case_dir = Path(sys.argv[2])
temp_manifest = Path(sys.argv[3])
source_manifest = root_dir / "data/input/figures/manifest_figures_2_to_6.csv"

temp_case_dir.mkdir(parents=True, exist_ok=True)

with source_manifest.open(newline="", encoding="utf-8") as manifest_handle:
    reader = csv.DictReader(manifest_handle)
    rows = []
    for row in reader:
        source_case_path = root_dir / row["case_file"]
        target_case_path = temp_case_dir / Path(row["case_file"]).name
        with source_case_path.open(encoding="utf-8") as case_handle:
            data = json.load(case_handle)

        if data.get("study", {}).get("kind", "dispersion_curve") == "field_map":
            data["discretization"]["Nx"] = 4
            data["discretization"]["Ny"] = 2
            data["field_map"]["sample_nx"] = 11
            data["field_map"]["sample_ny"] = 7
        else:
            data["discretization"]["Nx"] = 4
            data["discretization"]["Ny"] = 2
            sweep = data["sweep"]
            sweep["v_end"] = min(sweep["v_end"], sweep["v_start"] + 2.0 * sweep["v_step"])

        with target_case_path.open("w", encoding="utf-8") as target_handle:
            json.dump(data, target_handle, indent=2)
            target_handle.write("\n")

        row["case_file"] = str(target_case_path)
        rows.append(row)

with temp_manifest.open("w", newline="", encoding="utf-8") as manifest_handle:
    writer = csv.DictWriter(
        manifest_handle,
        fieldnames=[
            "figure_id",
            "case_id",
            "case_file",
            "study_kind",
            "curve_id",
            "canonical_csv_name",
            "status",
        ],
    )
    writer.writeheader()
    writer.writerows(rows)
PY

"${ROOT_DIR}/scripts/generate_figures_2_to_6_csvs.sh" "${OUTPUT_DIR}" "${TEMP_MANIFEST}"
"${ROOT_DIR}/scripts/run_case.sh" "${TEMP_CASE_DIR}/fig_05_field_map_preparation.json"

INDEX_CSV="${OUTPUT_DIR}/figures_2_to_6_index.csv"
FIG02_DIR="${OUTPUT_DIR}/fig_02/fig_02_homogeneous_rectangular_eq_integral"
FIG05_DIR="${OUTPUT_DIR}/fig_05/fig_05_field_map_preparation"

test -f "${INDEX_CSV}"
test "$(wc -l < "${INDEX_CSV}")" -eq 8

test -f "${FIG02_DIR}/dispersion_curve.csv"
test -f "${FIG02_DIR}/results.csv"
test -f "${FIG02_DIR}/output_manifest.json"
test -f "${FIG02_DIR}/performance_summary.json"
test -f "${FIG02_DIR}/fig_02_integral_equation_comparison.csv"
test -f "${FIG02_DIR}/fig_02_integral_equation_metrics.json"
test -f "${FIG02_DIR}/fig_02_integral_equation_overlay.png"
test -f "${FIG02_DIR}/mode_coefficients.csv" || true
test -f "${FIG05_DIR}/field_map.csv"
test -f "${FIG05_DIR}/field_sampling_grid.csv"
test -f "${FIG05_DIR}/field_map_status.txt"
test -f "${FIG05_DIR}/results.csv"
test -f "${FIG05_DIR}/output_manifest.json"
test -f "${FIG05_DIR}/performance_summary.json"
test -f "${DEFAULT_FIG05_DIR}/field_map.csv"
test -f "${DEFAULT_FIG05_DIR}/output_manifest.json"
test -f "${DEFAULT_FIG05_DIR}/performance_summary.json"

rg -q "^case_id,article_figure,target_mode,article_x_param,lambda0,beta,beta_over_k0,normalized_beta,det_abs,modal_residual$" \
    "${FIG02_DIR}/dispersion_curve.csv"
rg -q "^article_x_param,normalized_beta_simulation,normalized_beta_reference,absolute_error,squared_error$" \
    "${FIG02_DIR}/fig_02_integral_equation_comparison.csv"
rg -q '"sample_count"' "${FIG02_DIR}/fig_02_integral_equation_metrics.json"
rg -q "^sample_id,x,y,refractive_index,k_squared,requested_component$" \
    "${FIG05_DIR}/field_sampling_grid.csv"
rg -q "^sample_id,x,y,refractive_index,k_squared,requested_component,field_real,field_imag,field_abs,field_abs_normalized,beta,beta_over_k0,modal_residual$" \
    "${FIG05_DIR}/field_map.csv"
rg -q '"status": "dispersion_csv_generated_with_det_search"' "${FIG02_DIR}/output_manifest.json"
rg -q '"status": "field_reconstructed_from_mode_solution"' "${FIG05_DIR}/output_manifest.json"
rg -q "\"output_dir\": \"${DEFAULT_FIG05_DIR}\"" "${DEFAULT_FIG05_DIR}/output_manifest.json"
rg -q '"performance_summary_path"' "${FIG02_DIR}/output_manifest.json"
rg -q '"performance_summary_path"' "${FIG05_DIR}/output_manifest.json"
rg -q '"self_green_singular_log_quadratures"' "${FIG02_DIR}/performance_summary.json"
rg -q '"self_green_singular_log_quadratures"' "${FIG05_DIR}/performance_summary.json"
rg -q "^status: reconstructed_from_mode_solution$" "${FIG05_DIR}/field_map_status.txt"

echo "Smoke test das figuras 2 a 6 concluído com sucesso em ${OUTPUT_DIR}"
