#!/usr/bin/env python3

import argparse
import csv
import json
import os
import shutil
import subprocess
import tempfile


def run_case(root_dir, executable, case_data, output_dir):
    case_path = os.path.join(output_dir, "case.json")
    with open(case_path, "w", encoding="utf-8") as handle:
        json.dump(case_data, handle, indent=2)

    subprocess.run([executable, case_path, output_dir], check=True, cwd=root_dir)

    results_path = os.path.join(output_dir, "results.csv")
    with open(results_path, newline="", encoding="utf-8") as handle:
        reader = csv.DictReader(handle)
        rows = list(reader)
    if len(rows) != 1:
        raise RuntimeError("A comparação de sensibilidade espera exatamente uma linha em results.csv.")
    return rows[0]


def main():
    parser = argparse.ArgumentParser(
        description="Compara a sensibilidade a malha e ao modelo de quadratura da fronteira."
    )
    parser.add_argument(
        "--base-case",
        default="data/input/fig4_parabolic_1d.json",
        help="Caso base em JSON.",
    )
    parser.add_argument(
        "--output-dir",
        default="out/boundary_sensitivity",
        help="Diretório onde os artefatos e o CSV resumido serão salvos.",
    )
    parser.add_argument(
        "--v-target",
        type=float,
        default=3.5,
        help="Valor único de V a ser usado na comparação.",
    )
    args = parser.parse_args()

    root_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
    executable = os.path.join(root_dir, "bin", "waveguide_solver")
    if not os.path.exists(executable):
        raise RuntimeError("Executável não encontrado. Rode scripts/build.sh antes.")

    with open(os.path.join(root_dir, args.base_case), encoding="utf-8") as handle:
        base_case = json.load(handle)

    os.makedirs(os.path.join(root_dir, args.output_dir), exist_ok=True)
    summary_path = os.path.join(root_dir, args.output_dir, "boundary_sensitivity.csv")
    temporary_root = tempfile.mkdtemp(prefix="pichot_boundary_sensitivity_")

    meshes = [(6, 3), (8, 4), (10, 5)]
    models = [("midpoint", 1), ("gauss2", 1), ("gauss2", 2), ("gauss2", 4)]

    rows = []
    try:
        for nx, ny in meshes:
            for model_name, subdivisions in models:
                case_data = json.loads(json.dumps(base_case))
                case_data["case_id"] = f"sensitivity_nx{nx}_ny{ny}_{model_name}_sub{subdivisions}"
                case_data["discretization"]["Nx"] = nx
                case_data["discretization"]["Ny"] = ny
                case_data["sweep"]["v_start"] = args.v_target
                case_data["sweep"]["v_end"] = args.v_target
                case_data["sweep"]["v_step"] = 1.0
                case_data.setdefault("solver", {})
                case_data["solver"]["include_boundary_distribution"] = True
                case_data["solver"]["boundary_quadrature_model"] = model_name
                case_data["solver"]["boundary_subdivisions"] = subdivisions
                case_output_dir = os.path.join(
                    temporary_root, f"nx{nx}_ny{ny}_{model_name}_sub{subdivisions}"
                )
                os.makedirs(case_output_dir, exist_ok=True)
                result = run_case(root_dir, executable, case_data, case_output_dir)
                rows.append(
                    {
                        "Nx": nx,
                        "Ny": ny,
                        "boundary_model": model_name,
                        "boundary_subdivisions": subdivisions,
                        "article_x_param": result["article_x_param"],
                        "beta": result["beta"],
                        "normalized_beta": result["normalized_beta"],
                        "det_abs": result["det_abs"],
                        "modal_residual": result["modal_residual"],
                    }
                )
                shutil.copytree(
                    case_output_dir,
                    os.path.join(
                        root_dir,
                        args.output_dir,
                        f"nx{nx}_ny{ny}_{model_name}_sub{subdivisions}",
                    ),
                    dirs_exist_ok=True,
                )
    finally:
        shutil.rmtree(temporary_root, ignore_errors=True)

    with open(summary_path, "w", newline="", encoding="utf-8") as handle:
        fieldnames = [
            "Nx",
            "Ny",
            "boundary_model",
            "boundary_subdivisions",
            "article_x_param",
            "beta",
            "normalized_beta",
            "det_abs",
            "modal_residual",
        ]
        writer = csv.DictWriter(handle, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(rows)

    print(f"Comparação salva em: {summary_path}")


if __name__ == "__main__":
    main()
