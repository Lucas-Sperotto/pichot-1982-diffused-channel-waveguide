#!/bin/bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
CASE_FILE="${ROOT_DIR}/data/input/smoke_homogeneous.json"
OUTPUT_DIR="$(mktemp -d /tmp/pichot_smoke_case.XXXXXX)"

"${ROOT_DIR}/scripts/build.sh"
"${ROOT_DIR}/bin/waveguide_solver" "${CASE_FILE}" "${OUTPUT_DIR}"

test -f "${OUTPUT_DIR}/results.csv"
test -f "${OUTPUT_DIR}/profile_samples.csv"
test -f "${OUTPUT_DIR}/input_snapshot.json"
test -f "${OUTPUT_DIR}/run_summary.txt"

rg -q "^case_id,article_figure,target_mode,V_param,lambda0,beta,B_norm,det_abs$" "${OUTPUT_DIR}/results.csv"
rg -q "^cell_id,x_center,y_center,dx,dy,refractive_index,k_squared$" "${OUTPUT_DIR}/profile_samples.csv"

echo "Smoke test concluído com sucesso em ${OUTPUT_DIR}"
