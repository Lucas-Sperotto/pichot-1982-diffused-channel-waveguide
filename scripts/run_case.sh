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
    OUTPUT_DIR="$(
        python3 -c '
import json
import sys
from pathlib import Path

case_file = Path(sys.argv[1])
root_dir = Path(sys.argv[2])
with case_file.open(encoding="utf-8") as f:
    data = json.load(f)

case_id = data["case_id"]
output = data.get("output", {})
family = output.get("family", "cases")
figure_id = output.get("figure_id", "")

if family == "figures" and figure_id:
    path = root_dir / "out" / "figures" / figure_id / case_id
else:
    path = root_dir / "out" / case_id

print(path)
' "${CASE_FILE}" "${ROOT_DIR}"
    )"
fi

mkdir -p "${OUTPUT_DIR}"
"${EXECUTABLE}" "${CASE_FILE}" "${OUTPUT_DIR}"

echo "Saídas disponíveis em: ${OUTPUT_DIR}"
