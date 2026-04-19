#!/bin/bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
CASE_FILE="${ROOT_DIR}/data/input/figures/fig_02_homogeneous_rectangular_eq_integral.json"
OUTPUT_DIR="${1:-${ROOT_DIR}/out/figures/fig_02/mesh_convergence}"
MESH_LEVELS="${FIG02_MESH_LEVELS:-4x2,8x4,12x6,16x8}"

if [[ "${OUTPUT_DIR}" != /* ]]; then
    OUTPUT_DIR="${ROOT_DIR}/${OUTPUT_DIR}"
fi

TEMP_CASE_DIR="$(mktemp -d /tmp/pichot_fig02_mesh_cases.XXXXXX)"
trap 'rm -rf "${TEMP_CASE_DIR}"' EXIT

"${ROOT_DIR}/scripts/build.sh"
mkdir -p "${OUTPUT_DIR}"

IFS=',' read -r -a MESH_ARRAY <<< "${MESH_LEVELS}"
for mesh in "${MESH_ARRAY[@]}"; do
    Nx="${mesh%x*}"
    Ny="${mesh#*x}"
    MESH_ID="$(printf 'Nx%02d_Ny%02d' "${Nx}" "${Ny}")"
    TEMP_CASE="${TEMP_CASE_DIR}/${MESH_ID}.json"

    python3 - <<'PY' "${CASE_FILE}" "${TEMP_CASE}" "${Nx}" "${Ny}"
import json
import os
import sys

source_path, target_path, nx_value, ny_value = sys.argv[1:5]
with open(source_path, encoding="utf-8") as handle:
    data = json.load(handle)

data["discretization"]["Nx"] = int(nx_value)
data["discretization"]["Ny"] = int(ny_value)

if "FIG02_SWEEP_START" in os.environ:
    data["sweep"]["v_start"] = float(os.environ["FIG02_SWEEP_START"])
if "FIG02_SWEEP_END" in os.environ:
    data["sweep"]["v_end"] = float(os.environ["FIG02_SWEEP_END"])
if "FIG02_SWEEP_STEP" in os.environ:
    data["sweep"]["v_step"] = float(os.environ["FIG02_SWEEP_STEP"])

with open(target_path, "w", encoding="utf-8") as handle:
    json.dump(data, handle, indent=2)
    handle.write("\n")
PY

    "${ROOT_DIR}/scripts/run_case.sh" "${TEMP_CASE}" "${OUTPUT_DIR}/${MESH_ID}"
done

python3 "${ROOT_DIR}/scripts/postprocess_fig02_mesh_convergence.py" --output-dir "${OUTPUT_DIR}"
echo "Convergência de malha da Figura 2 salva em: ${OUTPUT_DIR}"
