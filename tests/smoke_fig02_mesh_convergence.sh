#!/bin/bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
OUTPUT_DIR="$(mktemp -d /tmp/pichot_fig02_mesh_conv.XXXXXX)"

FIG02_MESH_LEVELS="4x2,8x4" \
FIG02_SWEEP_START="0.6" \
FIG02_SWEEP_END="1.4" \
FIG02_SWEEP_STEP="0.4" \
    "${ROOT_DIR}/scripts/run_fig02_mesh_convergence.sh" "${OUTPUT_DIR}"

test -f "${OUTPUT_DIR}/fig_02_mesh_convergence_summary.csv"
test -f "${OUTPUT_DIR}/fig_02_mesh_convergence_metrics.json"
test -f "${OUTPUT_DIR}/fig_02_mesh_convergence_overlay.png"
test -f "${OUTPUT_DIR}/Nx04_Ny02/dispersion_curve.csv"
test -f "${OUTPUT_DIR}/Nx04_Ny02/validation_manifest.json"
test -f "${OUTPUT_DIR}/Nx08_Ny04/dispersion_curve.csv"
test -f "${OUTPUT_DIR}/Nx08_Ny04/validation_manifest.json"

rg -q "^mesh_id,Nx,Ny,comparison_target_kind,comparison_target_id,comparison_target_label,sample_count,x_min,x_max,mean_absolute_error,max_absolute_error,root_mean_squared_error$" \
    "${OUTPUT_DIR}/fig_02_mesh_convergence_summary.csv"
rg -q '"baseline_mesh_id": "Nx08_Ny04"' "${OUTPUT_DIR}/fig_02_mesh_convergence_metrics.json"
rg -q '"mesh_count": 2' "${OUTPUT_DIR}/fig_02_mesh_convergence_metrics.json"

echo "Smoke test da convergência de malha da Figura 2 concluído com sucesso em ${OUTPUT_DIR}"
