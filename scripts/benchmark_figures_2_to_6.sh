#!/bin/bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
TIMESTAMP="$(date +%Y%m%d_%H%M%S)"
OUTPUT_ROOT="${1:-${ROOT_DIR}/out/benchmarks/figures_2_to_6/${TIMESTAMP}}"
MANIFEST_INPUT="${2:-${ROOT_DIR}/data/input/figures/manifest_figures_2_to_6.csv}"

if [[ "${OUTPUT_ROOT}" != /* ]]; then
    OUTPUT_ROOT="${ROOT_DIR}/${OUTPUT_ROOT}"
fi

if [[ "${MANIFEST_INPUT}" != /* ]]; then
    MANIFEST_INPUT="${ROOT_DIR}/${MANIFEST_INPUT}"
fi

if [[ ! -f "${MANIFEST_INPUT}" ]]; then
    echo "Manifest de benchmark não encontrado: ${MANIFEST_INPUT}" >&2
    exit 1
fi

"${ROOT_DIR}/scripts/build.sh"

mkdir -p "${OUTPUT_ROOT}"
BENCHMARK_CASES_CSV="${OUTPUT_ROOT}/benchmark_cases.csv"
BENCHMARK_SUMMARY_JSON="${OUTPUT_ROOT}/benchmark_summary.json"

echo "figure_id,case_id,study_kind,curve_id,output_dir,wall_seconds,solver_wall_seconds,performance_summary_path" \
    > "${BENCHMARK_CASES_CSV}"

total_start_ns="$(date +%s%N)"

while IFS=, read -r FIGURE_ID CASE_ID CASE_PATH STUDY_KIND CURVE_ID CANONICAL_CSV SOURCE_STATUS; do
    if [[ -z "${CASE_ID}" ]]; then
        continue
    fi

    RESOLVED_CASE_PATH="${CASE_PATH}"
    if [[ "${RESOLVED_CASE_PATH}" != /* ]]; then
        RESOLVED_CASE_PATH="${ROOT_DIR}/${RESOLVED_CASE_PATH}"
    fi

    if [[ ! -f "${RESOLVED_CASE_PATH}" ]]; then
        echo "Caso listado no manifest não encontrado: ${CASE_PATH}" >&2
        exit 1
    fi

    OUT_DIR="${OUTPUT_ROOT}/${FIGURE_ID}/${CASE_ID}"
    case_start_ns="$(date +%s%N)"
    "${ROOT_DIR}/scripts/run_case.sh" "${RESOLVED_CASE_PATH}" "${OUT_DIR}"
    case_end_ns="$(date +%s%N)"

    PERFORMANCE_JSON="${OUT_DIR}/performance_summary.json"
    if [[ ! -f "${PERFORMANCE_JSON}" ]]; then
        echo "Arquivo performance_summary.json não encontrado para ${CASE_ID}." >&2
        exit 1
    fi

    CASE_WALL_SECONDS="$(
        python3 - <<'PY' "${case_start_ns}" "${case_end_ns}"
import sys

start_ns = int(sys.argv[1])
end_ns = int(sys.argv[2])
print((end_ns - start_ns) / 1e9)
PY
    )"

    SOLVER_WALL_SECONDS="$(
        python3 - <<'PY' "${PERFORMANCE_JSON}"
import json
import sys

with open(sys.argv[1], encoding="utf-8") as handle:
    print(json.load(handle)["solver_wall_seconds"])
PY
    )"

    echo "${FIGURE_ID},${CASE_ID},${STUDY_KIND},${CURVE_ID},${OUT_DIR},${CASE_WALL_SECONDS},${SOLVER_WALL_SECONDS},${PERFORMANCE_JSON}" \
        >> "${BENCHMARK_CASES_CSV}"
done < <(tail -n +2 "${MANIFEST_INPUT}")

total_end_ns="$(date +%s%N)"
TOTAL_WALL_SECONDS="$(
    python3 - <<'PY' "${total_start_ns}" "${total_end_ns}"
import sys

start_ns = int(sys.argv[1])
end_ns = int(sys.argv[2])
print((end_ns - start_ns) / 1e9)
PY
)"

python3 - <<'PY' "${OUTPUT_ROOT}" "${MANIFEST_INPUT}" "${BENCHMARK_CASES_CSV}" "${BENCHMARK_SUMMARY_JSON}" "${TOTAL_WALL_SECONDS}"
import csv
import json
import sys
from pathlib import Path

output_root = Path(sys.argv[1])
manifest_input = Path(sys.argv[2])
cases_csv = Path(sys.argv[3])
summary_json = Path(sys.argv[4])
total_wall_seconds = float(sys.argv[5])

rows = []
green_totals = {
    "g_ns_value_requests": 0,
    "g_ns_dx_requests": 0,
    "g_ns_dy_requests": 0,
    "g_ns_bundle_evaluations": 0,
    "oscillatory_branch_evaluations": 0,
    "transformed_branch_evaluations": 0,
}
matrix_totals = {
    "shared_volume_bundle_evaluations": 0,
    "shared_boundary_bundle_evaluations": 0,
    "self_green_regularizations": 0,
    "self_green_singular_log_quadratures": 0,
    "self_dy_regularizations": 0,
}
solver_wall_seconds_sum = 0.0

with cases_csv.open(newline="", encoding="utf-8") as handle:
    reader = csv.DictReader(handle)
    for row in reader:
        rows.append(row)
        perf_path = Path(row["performance_summary_path"])
        with perf_path.open(encoding="utf-8") as perf_handle:
            perf = json.load(perf_handle)
        solver_wall_seconds_sum += float(perf["solver_wall_seconds"])
        for key in green_totals:
            green_totals[key] += int(perf["green_function"][key])
        for key in matrix_totals:
            matrix_totals[key] += int(perf["matrix_solver"][key])

summary = {
    "manifest_input": str(manifest_input),
    "output_root": str(output_root),
    "case_count": len(rows),
    "total_wall_seconds": total_wall_seconds,
    "solver_wall_seconds_sum": solver_wall_seconds_sum,
    "benchmark_cases_csv_path": str(cases_csv),
    "green_function_totals": green_totals,
    "matrix_solver_totals": matrix_totals,
}

with summary_json.open("w", encoding="utf-8") as handle:
    json.dump(summary, handle, indent=2)
    handle.write("\n")
PY

echo "Benchmark consolidado em: ${BENCHMARK_SUMMARY_JSON}"
