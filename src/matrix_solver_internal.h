#pragma once

#include <cstdint>

#include "matrix_solver.h"

struct MatrixSolverPerformanceStats {
    std::uint64_t shared_volume_bundle_evaluations = 0;
    std::uint64_t shared_boundary_bundle_evaluations = 0;
    std::uint64_t self_green_regularizations = 0;
    std::uint64_t self_green_singular_log_quadratures = 0;
    std::uint64_t self_dy_regularizations = 0;
};

MatrixSolverPerformanceStats get_matrix_solver_performance_stats();
void reset_matrix_solver_performance_stats();

Complex calculate_self_cell_green_singular_average(const Cell& cell,
                                                   double beta,
                                                   const Waveguide& wg);
