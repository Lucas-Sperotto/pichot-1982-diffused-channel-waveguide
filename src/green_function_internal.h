#pragma once

#include <cstdint>

#include "green_function.h"

struct GreenFunctionNonSingularBundle {
    Complex value{0.0, 0.0};
    Complex d_dx_source{0.0, 0.0};
    Complex d_dy_source{0.0, 0.0};
};

struct GreenFunctionPerformanceStats {
    std::uint64_t g_ns_value_requests = 0;
    std::uint64_t g_ns_dx_requests = 0;
    std::uint64_t g_ns_dy_requests = 0;
    std::uint64_t g_ns_bundle_evaluations = 0;
    std::uint64_t oscillatory_branch_evaluations = 0;
    std::uint64_t transformed_branch_evaluations = 0;
};

GreenFunctionNonSingularBundle calculate_G_NS_bundle_internal(
    double x,
    double y,
    double xp,
    double yp,
    double beta,
    const Waveguide& wg);

GreenFunctionPerformanceStats get_green_function_performance_stats();
void reset_green_function_performance_stats();
