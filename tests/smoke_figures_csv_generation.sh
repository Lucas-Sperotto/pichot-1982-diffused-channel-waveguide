#!/bin/bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
OUTPUT_DIR="$(mktemp -d /tmp/pichot_figures_csvs.XXXXXX)"
DEFAULT_FIG05_DIR="${ROOT_DIR}/out/figures/fig_05/fig_05_field_map_preparation"

"${ROOT_DIR}/scripts/generate_figures_2_to_6_csvs.sh" "${OUTPUT_DIR}"
"${ROOT_DIR}/scripts/run_case.sh" "data/input/figures/fig_05_field_map_preparation.json"

INDEX_CSV="${OUTPUT_DIR}/figures_2_to_6_index.csv"
FIG02_DIR="${OUTPUT_DIR}/fig_02/fig_02_homogeneous_rectangular_eq_integral"
FIG05_DIR="${OUTPUT_DIR}/fig_05/fig_05_field_map_preparation"

test -f "${INDEX_CSV}"
test "$(wc -l < "${INDEX_CSV}")" -eq 8

test -f "${FIG02_DIR}/dispersion_curve.csv"
test -f "${FIG02_DIR}/results.csv"
test -f "${FIG02_DIR}/output_manifest.json"
test -f "${FIG02_DIR}/mode_coefficients.csv" || true
test -f "${FIG05_DIR}/field_map.csv"
test -f "${FIG05_DIR}/field_sampling_grid.csv"
test -f "${FIG05_DIR}/field_map_status.txt"
test -f "${FIG05_DIR}/results.csv"
test -f "${FIG05_DIR}/output_manifest.json"
test -f "${DEFAULT_FIG05_DIR}/field_map.csv"
test -f "${DEFAULT_FIG05_DIR}/output_manifest.json"

rg -q "^case_id,article_figure,target_mode,article_x_param,lambda0,beta,beta_over_k0,normalized_beta,det_abs,modal_residual$" \
    "${FIG02_DIR}/dispersion_curve.csv"
rg -q "^sample_id,x,y,refractive_index,k_squared,requested_component$" \
    "${FIG05_DIR}/field_sampling_grid.csv"
rg -q "^sample_id,x,y,refractive_index,k_squared,requested_component,field_real,field_imag,field_abs,field_abs_normalized,beta,beta_over_k0,modal_residual$" \
    "${FIG05_DIR}/field_map.csv"
rg -q '"status": "dispersion_csv_generated_with_modal_residual"' "${FIG02_DIR}/output_manifest.json"
rg -q '"status": "field_reconstructed_from_mode_solution"' "${FIG05_DIR}/output_manifest.json"
rg -q "\"output_dir\": \"${DEFAULT_FIG05_DIR}\"" "${DEFAULT_FIG05_DIR}/output_manifest.json"
rg -q "^status: reconstructed_from_mode_solution$" "${FIG05_DIR}/field_map_status.txt"

echo "Smoke test das figuras 2 a 6 concluído com sucesso em ${OUTPUT_DIR}"
