#!/bin/bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
BIN_DIR="${ROOT_DIR}/bin"
SRC_DIR="${ROOT_DIR}/src"
EXECUTABLE="${BIN_DIR}/waveguide_solver"

mkdir -p "${BIN_DIR}"

echo "Compilando o projeto..."
g++ -std=c++17 -O2 -Wall -Wextra -pedantic \
    "${SRC_DIR}/main.cpp" \
    "${SRC_DIR}/case_io.cpp" \
    "${SRC_DIR}/waveguide.cpp" \
    "${SRC_DIR}/green_function.cpp" \
    "${SRC_DIR}/matrix_solver.cpp" \
    -o "${EXECUTABLE}"

echo "Compilação bem-sucedida: ${EXECUTABLE}"
