#!/bin/bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
CASE_FILE="${ROOT_DIR}/data/input/smoke_homogeneous.json"
OUTPUT_DIR="$(mktemp -d /tmp/pichot_smoke_case.XXXXXX)"
TEMP_CASE="$(mktemp /tmp/pichot_smoke_case_json.XXXXXX.json)"

python3 - <<'PY' "${CASE_FILE}" "${TEMP_CASE}"
import json
import sys

source_path, target_path = sys.argv[1:3]
with open(source_path, encoding="utf-8") as handle:
    data = json.load(handle)

data["discretization"]["Nx"] = 4
data["discretization"]["Ny"] = 2
data["sweep"]["v_start"] = 1.0
data["sweep"]["v_end"] = 1.0
data["sweep"]["v_step"] = 1.0

with open(target_path, "w", encoding="utf-8") as handle:
    json.dump(data, handle, indent=2)
    handle.write("\n")
PY

"${ROOT_DIR}/scripts/build.sh"
"${ROOT_DIR}/bin/waveguide_solver" "${TEMP_CASE}" "${OUTPUT_DIR}"

test -f "${OUTPUT_DIR}/results.csv"
test -f "${OUTPUT_DIR}/profile_samples.csv"
test -f "${OUTPUT_DIR}/input_snapshot.json"
test -f "${OUTPUT_DIR}/run_summary.txt"
test -f "${OUTPUT_DIR}/output_manifest.json"
test -f "${OUTPUT_DIR}/performance_summary.json"

rg -q "^case_id,article_figure,target_mode,article_x_param,lambda0,beta,beta_over_k0,normalized_beta,det_abs,modal_residual$" "${OUTPUT_DIR}/results.csv"
rg -q "^cell_id,x_center,y_center,dx,dy,refractive_index,k_squared$" "${OUTPUT_DIR}/profile_samples.csv"
rg -q '"performance_summary_path"' "${OUTPUT_DIR}/output_manifest.json"
rg -q '"solver_wall_seconds"' "${OUTPUT_DIR}/performance_summary.json"

echo "Smoke test concluído com sucesso em ${OUTPUT_DIR}"
