#!/bin/bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
CASE_FILE="${ROOT_DIR}/data/input/figures/fig_02_homogeneous_rectangular_eq_integral.json"
OUTPUT_DIR="$(mktemp -d /tmp/pichot_fig02_overlay.XXXXXX)"
TEMP_CASE="$(mktemp /tmp/pichot_fig02_overlay_json.XXXXXX.json)"

python3 - <<'PY' "${CASE_FILE}" "${TEMP_CASE}"
import json
import sys

source_path, target_path = sys.argv[1:3]
with open(source_path, encoding="utf-8") as handle:
    data = json.load(handle)

data["discretization"]["Nx"] = 4
data["discretization"]["Ny"] = 2
data["sweep"]["v_start"] = 0.6
data["sweep"]["v_end"] = 1.4
data["sweep"]["v_step"] = 0.4

with open(target_path, "w", encoding="utf-8") as handle:
    json.dump(data, handle, indent=2)
    handle.write("\n")
PY

"${ROOT_DIR}/scripts/build.sh"
"${ROOT_DIR}/scripts/run_case.sh" "${TEMP_CASE}" "${OUTPUT_DIR}"

test -f "${OUTPUT_DIR}/dispersion_curve.csv"
test -f "${OUTPUT_DIR}/fig_02_integral_equation_comparison.csv"
test -f "${OUTPUT_DIR}/fig_02_integral_equation_metrics.json"
test -f "${OUTPUT_DIR}/fig_02_integral_equation_overlay.png"
test -f "${OUTPUT_DIR}/fig_02_goell_comparison.csv"
test -f "${OUTPUT_DIR}/fig_02_goell_metrics.json"
test -f "${OUTPUT_DIR}/fig_02_yeh_fem_comparison.csv"
test -f "${OUTPUT_DIR}/fig_02_yeh_fem_metrics.json"
test -f "${OUTPUT_DIR}/fig_02_effective_index_comparison.csv"
test -f "${OUTPUT_DIR}/fig_02_effective_index_metrics.json"
test -f "${OUTPUT_DIR}/fig_02_marcatili_digitized_comparison.csv"
test -f "${OUTPUT_DIR}/fig_02_marcatili_digitized_metrics.json"
test -f "${OUTPUT_DIR}/fig_02_marcatili_external_comparison.csv"
test -f "${OUTPUT_DIR}/fig_02_marcatili_external_metrics.json"
test -f "${OUTPUT_DIR}/fig_02_validation_overlay.png"
test -f "${OUTPUT_DIR}/validation_manifest.json"
test -f "${OUTPUT_DIR}/performance_summary.json"
test -f "${ROOT_DIR}/data/reference/fig_02_integral_equation_digitized.csv"
test -f "${ROOT_DIR}/data/reference/fig_02_references_manifest.json"
test -f "${ROOT_DIR}/data/reference/fig_02_goell_digitized.csv"
test -f "${ROOT_DIR}/data/reference/fig_02_yeh_fem_digitized.csv"
test -f "${ROOT_DIR}/data/reference/fig_02_effective_index_digitized.csv"
test -f "${ROOT_DIR}/data/reference/fig_02_marcatili_digitized.csv"
test -f "${ROOT_DIR}/data/reference/fig_02_marcatili_external.csv"

rg -q "^article_x_param,normalized_beta$" "${ROOT_DIR}/data/reference/fig_02_integral_equation_digitized.csv"
rg -q '"reference_id": "marcatili_external"' "${ROOT_DIR}/data/reference/fig_02_references_manifest.json"
rg -q "^article_x_param,normalized_beta_simulation,normalized_beta_reference,absolute_error,squared_error$" \
    "${OUTPUT_DIR}/fig_02_integral_equation_comparison.csv"
rg -q '"reference_curve": "integral_equation"' "${OUTPUT_DIR}/fig_02_integral_equation_metrics.json"
rg -q '"reference_curve": "goell"' "${OUTPUT_DIR}/fig_02_goell_metrics.json"
rg -q '"reference_curve": "yeh_fem"' "${OUTPUT_DIR}/fig_02_yeh_fem_metrics.json"
rg -q '"reference_curve": "effective_index"' "${OUTPUT_DIR}/fig_02_effective_index_metrics.json"
rg -q '"reference_curve": "marcatili_digitized"' "${OUTPUT_DIR}/fig_02_marcatili_digitized_metrics.json"
rg -q '"reference_curve": "marcatili_external"' "${OUTPUT_DIR}/fig_02_marcatili_external_metrics.json"
rg -q '"validation_overlay_png"' "${OUTPUT_DIR}/validation_manifest.json"
rg -q '"reference_id": "goell"' "${OUTPUT_DIR}/validation_manifest.json"
rg -q '"reference_id": "marcatili_external"' "${OUTPUT_DIR}/validation_manifest.json"
rg -q '"solver_wall_seconds"' "${OUTPUT_DIR}/performance_summary.json"
rg -q '"oscillatory_branch_evaluations"' "${OUTPUT_DIR}/performance_summary.json"

echo "Smoke test do overlay da Figura 2 concluído com sucesso em ${OUTPUT_DIR}"
