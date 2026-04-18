#!/bin/bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
BIN_DIR="${ROOT_DIR}/bin"
SRC_DIR="${ROOT_DIR}/src"
TEST_BIN="${BIN_DIR}/test_matrix_solver"

mkdir -p "${BIN_DIR}"

g++ -std=c++17 -O2 -Wall -Wextra -pedantic -I"${SRC_DIR}" \
    "${ROOT_DIR}/tests/test_matrix_solver.cpp" \
    "${SRC_DIR}/matrix_solver.cpp" \
    "${SRC_DIR}/green_function.cpp" \
    "${SRC_DIR}/waveguide.cpp" \
    -o "${TEST_BIN}"

"${TEST_BIN}"
