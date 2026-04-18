#!/bin/bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
CASE_FILE="${1:-data/input/figures/fig_04_curve_A_diffused_1d_eq_integral.json}"
OUTPUT_DIR="${2:-}"

if [[ "${CASE_FILE}" == "clean" ]]; then
    echo "Limpando diretórios bin/ e out/..."
    rm -rf "${ROOT_DIR}/bin" "${ROOT_DIR}/out"
    echo "Limpeza concluída."
    exit 0
fi

"${ROOT_DIR}/scripts/build.sh"

if [[ -n "${OUTPUT_DIR}" ]]; then
    "${ROOT_DIR}/scripts/run_case.sh" "${CASE_FILE}" "${OUTPUT_DIR}"
else
    "${ROOT_DIR}/scripts/run_case.sh" "${CASE_FILE}"
fi
