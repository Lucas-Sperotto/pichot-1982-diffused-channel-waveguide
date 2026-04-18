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
test -f "${ROOT_DIR}/data/reference/fig_02_integral_equation_digitized.csv"

rg -q "^article_x_param,normalized_beta$" "${ROOT_DIR}/data/reference/fig_02_integral_equation_digitized.csv"
rg -q "^article_x_param,normalized_beta_simulation,normalized_beta_reference,absolute_error,squared_error$" \
    "${OUTPUT_DIR}/fig_02_integral_equation_comparison.csv"
rg -q '"reference_curve": "fig_02_integral_equation_digitized"' \
    "${OUTPUT_DIR}/fig_02_integral_equation_metrics.json"

echo "Smoke test do overlay da Figura 2 concluído com sucesso em ${OUTPUT_DIR}"
