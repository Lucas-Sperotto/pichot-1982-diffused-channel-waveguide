#!/bin/bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
EXECUTABLE="${ROOT_DIR}/bin/waveguide_solver"

if [[ $# -lt 1 ]]; then
    echo "Uso: $0 <arquivo_de_caso.json> [diretorio_saida]" >&2
    exit 1
fi

CASE_FILE="$1"
if [[ "${CASE_FILE}" != /* ]]; then
    CASE_FILE="${ROOT_DIR}/${CASE_FILE}"
fi

if [[ ! -f "${CASE_FILE}" ]]; then
    echo "Arquivo de caso não encontrado: ${CASE_FILE}" >&2
    exit 1
fi

if [[ $# -ge 2 ]]; then
    OUTPUT_DIR="$2"
    if [[ "${OUTPUT_DIR}" != /* ]]; then
        OUTPUT_DIR="${ROOT_DIR}/${OUTPUT_DIR}"
    fi
else
    CASE_NAME="$(basename "${CASE_FILE}" .json)"
    OUTPUT_DIR="${ROOT_DIR}/out/${CASE_NAME}"
fi

mkdir -p "${OUTPUT_DIR}"
"${EXECUTABLE}" "${CASE_FILE}" "${OUTPUT_DIR}"

echo "Saídas disponíveis em: ${OUTPUT_DIR}"
