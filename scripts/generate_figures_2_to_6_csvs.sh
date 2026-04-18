#!/bin/bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
OUTPUT_ROOT="${1:-${ROOT_DIR}/out/figures}"
MANIFEST_INPUT="${ROOT_DIR}/data/input/figures/manifest_figures_2_to_6.csv"

"${ROOT_DIR}/scripts/build.sh"

INDEX_CSV="${OUTPUT_ROOT}/figures_2_to_6_index.csv"
mkdir -p "${OUTPUT_ROOT}"
echo "figure_id,case_id,study_kind,curve_id,output_dir,canonical_csv,source_status,generation_status" > "${INDEX_CSV}"

while IFS=, read -r FIGURE_ID CASE_ID CASE_PATH STUDY_KIND CURVE_ID CANONICAL_CSV SOURCE_STATUS; do
    if [[ -z "${CASE_ID}" ]]; then
        continue
    fi

    if [[ ! -f "${ROOT_DIR}/${CASE_PATH}" ]]; then
        echo "Caso listado no manifest não encontrado: ${CASE_PATH}" >&2
        exit 1
    fi

    OUT_DIR="${OUTPUT_ROOT}/${FIGURE_ID}/${CASE_ID}"
    "${ROOT_DIR}/scripts/run_case.sh" "${CASE_PATH}" "${OUT_DIR}"
    echo "${FIGURE_ID},${CASE_ID},${STUDY_KIND},${CURVE_ID},${OUT_DIR},${CANONICAL_CSV},${SOURCE_STATUS},generated" >> "${INDEX_CSV}"
done < <(tail -n +2 "${MANIFEST_INPUT}")

echo "Índice salvo em: ${INDEX_CSV}"
