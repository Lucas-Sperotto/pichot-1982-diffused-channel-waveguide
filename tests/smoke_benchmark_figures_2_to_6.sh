#!/bin/bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
OUTPUT_DIR="$(mktemp -d /tmp/pichot_benchmark_figures.XXXXXX)"
TEMP_CASE_DIR="$(mktemp -d /tmp/pichot_benchmark_cases.XXXXXX)"
TEMP_MANIFEST="${TEMP_CASE_DIR}/manifest_figures_2_to_6.csv"

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

rows = []
with source_manifest.open(newline="", encoding="utf-8") as manifest_handle:
    reader = csv.DictReader(manifest_handle)
    for index, row in enumerate(reader):
        if index >= 3:
            break
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

"${ROOT_DIR}/scripts/benchmark_figures_2_to_6.sh" "${OUTPUT_DIR}" "${TEMP_MANIFEST}"

test -f "${OUTPUT_DIR}/benchmark_cases.csv"
test -f "${OUTPUT_DIR}/benchmark_summary.json"
test -f "${OUTPUT_DIR}/fig_02/fig_02_homogeneous_rectangular_eq_integral/performance_summary.json"

rg -q "^figure_id,case_id,study_kind,curve_id,output_dir,wall_seconds,solver_wall_seconds,performance_summary_path$" \
    "${OUTPUT_DIR}/benchmark_cases.csv"
rg -q '"total_wall_seconds"' "${OUTPUT_DIR}/benchmark_summary.json"
rg -q '"green_function_totals"' "${OUTPUT_DIR}/benchmark_summary.json"
rg -q '"matrix_solver_totals"' "${OUTPUT_DIR}/benchmark_summary.json"

echo "Smoke test do benchmark das Figuras 2 a 6 concluído com sucesso em ${OUTPUT_DIR}"
