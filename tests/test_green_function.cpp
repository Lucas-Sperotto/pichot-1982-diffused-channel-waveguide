#include <algorithm>
#include <cmath>
#include <complex>
#include <iostream>
#include <stdexcept>

#include "green_function.h"
#include "green_function_internal.h"
#include "waveguide.h"

namespace {

bool nearly_equal(double a, double b, double absolute_tolerance, double relative_tolerance) {
    const double scale = std::max(std::abs(a), std::abs(b));
    return std::abs(a - b) <= absolute_tolerance + relative_tolerance * scale;
}

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

Waveguide make_reference_waveguide() {
    WaveguideParams params;
    params.n1 = 1.0;
    params.n3 = 1.47;
    params.n2m = 1.49;
    params.a = 2.22e-6;
    params.b = 1.11e-6;
    params.lambda0 = 0.6328e-6;
    params.profile_type = ProfileType::PARABOLIC_1D;

    Discretization discretization;
    discretization.Nx = 10;
    discretization.Ny = 5;

    return Waveguide(params, discretization);
}

void test_green_symmetry_and_decay() {
    const Waveguide wg = make_reference_waveguide();
    const double beta = 1.01 * wg.get_k0() * wg.get_params().n2m;

    const double x = 0.15e-6;
    const double y = 0.45e-6;
    const double xp = -0.22e-6;
    const double yp = 0.70e-6;

    const Complex gs = calculate_G_S(x, y, xp, yp, beta, wg);
    const Complex gns = calculate_G_NS(x, y, xp, yp, beta, wg);
    const Complex g_total = gs + gns;
    const Complex g_swapped = calculate_G(xp, yp, x, y, beta, wg);

    require(std::isfinite(gs.real()), "G^S deveria ser finita fora da singularidade.");
    require(std::isfinite(gns.real()), "G^NS deveria ser finita no regime guiado.");
    require(std::abs(gs.imag()) < 1e-14, "G^S deveria ser real no regime suportado.");
    require(std::abs(gns.imag()) < 1e-14, "G^NS deveria ser real no regime suportado.");
    require(nearly_equal(g_total.real(), g_swapped.real(), 5e-8, 5e-6),
            "A função de Green deveria ser simétrica ao trocar observação e fonte.");

    const double y_near = 0.25e-6;
    const double y_far = 1.25e-6;
    const double magnitude_near = std::abs(calculate_G(x, y_near, xp, yp, beta, wg));
    const double magnitude_far = std::abs(calculate_G(x, y_far, xp, yp, beta, wg));
    require(magnitude_far < magnitude_near,
            "A magnitude da Green deveria decair ao afastar o ponto de observação da fonte.");
}

void test_green_gradients_against_finite_difference() {
    const Waveguide wg = make_reference_waveguide();
    const double beta = 1.01 * wg.get_k0() * wg.get_params().n2m;

    const double x = 0.12e-6;
    const double y = 0.40e-6;
    const double xp = -0.18e-6;
    const double yp = 0.62e-6;
    const double h = 2e-9;

    const double numerical_dx =
        (calculate_G(x, y, xp + h, yp, beta, wg).real() - calculate_G(x, y, xp - h, yp, beta, wg).real()) /
        (2.0 * h);
    const double numerical_dy =
        (calculate_G(x, y, xp, yp + h, beta, wg).real() - calculate_G(x, y, xp, yp - h, beta, wg).real()) /
        (2.0 * h);

    const double analytical_dx =
        (calculate_dG_S_dx_source(x, y, xp, yp, beta, wg) + calculate_dG_NS_dx_source(x, y, xp, yp, beta, wg))
            .real();
    const double analytical_dy =
        (calculate_dG_S_dy_source(x, y, xp, yp, beta, wg) + calculate_dG_NS_dy_source(x, y, xp, yp, beta, wg))
            .real();

    require(nearly_equal(analytical_dx, numerical_dx, 2e-2, 2e-3),
            "A derivada em x' da Green não bateu com a diferença finita.");
    require(nearly_equal(analytical_dy, numerical_dy, 2e-2, 2e-3),
            "A derivada em y' da Green não bateu com a diferença finita.");
}

void test_green_ns_gradients_against_finite_difference() {
    const Waveguide wg = make_reference_waveguide();
    const double beta = 1.01 * wg.get_k0() * wg.get_params().n2m;

    const double x = 0.12e-6;
    const double y = 0.40e-6;
    const double xp = -0.18e-6;
    const double yp = 0.62e-6;
    const double h = 2e-9;

    const double numerical_dx =
        (calculate_G_NS(x, y, xp + h, yp, beta, wg).real() - calculate_G_NS(x, y, xp - h, yp, beta, wg).real()) /
        (2.0 * h);
    const double numerical_dy =
        (calculate_G_NS(x, y, xp, yp + h, beta, wg).real() - calculate_G_NS(x, y, xp, yp - h, beta, wg).real()) /
        (2.0 * h);

    const double analytical_dx = calculate_dG_NS_dx_source(x, y, xp, yp, beta, wg).real();
    const double analytical_dy = calculate_dG_NS_dy_source(x, y, xp, yp, beta, wg).real();

    require(nearly_equal(analytical_dx, numerical_dx, 2e-2, 2e-3),
            "A derivada em x' de G^NS não bateu com a diferença finita.");
    require(nearly_equal(analytical_dy, numerical_dy, 1e-1, 1e-2),
            "A derivada em y' de G^NS não bateu com a diferença finita.");
}

void test_ns_bundle_matches_scalar_wrappers_off_axis() {
    const Waveguide wg = make_reference_waveguide();
    const double beta = 1.01 * wg.get_k0() * wg.get_params().n2m;

    const double x = 0.12e-6;
    const double y = 0.40e-6;
    const double xp = -0.18e-6;
    const double yp = 0.62e-6;

    const GreenFunctionNonSingularBundle bundle = calculate_G_NS_bundle_internal(x, y, xp, yp, beta, wg);
    const Complex scalar_value = calculate_G_NS(x, y, xp, yp, beta, wg);
    const Complex scalar_dx = calculate_dG_NS_dx_source(x, y, xp, yp, beta, wg);
    const Complex scalar_dy = calculate_dG_NS_dy_source(x, y, xp, yp, beta, wg);

    require(nearly_equal(bundle.value.real(), scalar_value.real(), 1e-12, 1e-12),
            "O bundle de G^NS deveria coincidir com o wrapper escalar fora do eixo x=x'.");
    require(nearly_equal(bundle.d_dx_source.real(), scalar_dx.real(), 1e-12, 1e-12),
            "O bundle de dG^NS/dx' deveria coincidir com o wrapper escalar fora do eixo x=x'.");
    require(nearly_equal(bundle.d_dy_source.real(), scalar_dy.real(), 1e-12, 1e-12),
            "O bundle de dG^NS/dy' deveria coincidir com o wrapper escalar fora do eixo x=x'.");
}

void test_ns_bundle_matches_scalar_wrappers_at_equal_abscissa() {
    const Waveguide wg = make_reference_waveguide();
    const double beta = 1.01 * wg.get_k0() * wg.get_params().n2m;

    const double x = 0.12e-6;
    const double y = 0.40e-6;
    const double xp = x;
    const double yp = 0.62e-6;

    const GreenFunctionNonSingularBundle bundle = calculate_G_NS_bundle_internal(x, y, xp, yp, beta, wg);
    const Complex scalar_value = calculate_G_NS(x, y, xp, yp, beta, wg);
    const Complex scalar_dx = calculate_dG_NS_dx_source(x, y, xp, yp, beta, wg);
    const Complex scalar_dy = calculate_dG_NS_dy_source(x, y, xp, yp, beta, wg);

    require(nearly_equal(bundle.value.real(), scalar_value.real(), 1e-12, 1e-12),
            "O bundle de G^NS deveria coincidir com o wrapper escalar no ramo x=x'.");
    require(nearly_equal(bundle.d_dx_source.real(), scalar_dx.real(), 1e-12, 1e-12),
            "O bundle de dG^NS/dx' deveria coincidir com o wrapper escalar no ramo x=x'.");
    require(nearly_equal(bundle.d_dy_source.real(), scalar_dy.real(), 1e-12, 1e-12),
            "O bundle de dG^NS/dy' deveria coincidir com o wrapper escalar no ramo x=x'.");
}

void test_green_function_stats_track_requests_and_branches() {
    const Waveguide wg = make_reference_waveguide();
    const double beta = 1.01 * wg.get_k0() * wg.get_params().n2m;

    reset_green_function_performance_stats();
    (void)calculate_G_NS_bundle_internal(0.12e-6, 0.40e-6, -0.18e-6, 0.62e-6, beta, wg);
    (void)calculate_G_NS_bundle_internal(0.12e-6, 0.40e-6, 0.12e-6, 0.62e-6, beta, wg);
    (void)calculate_G_NS(0.12e-6, 0.40e-6, -0.18e-6, 0.62e-6, beta, wg);
    (void)calculate_dG_NS_dx_source(0.12e-6, 0.40e-6, -0.18e-6, 0.62e-6, beta, wg);
    (void)calculate_dG_NS_dy_source(0.12e-6, 0.40e-6, 0.12e-6, 0.62e-6, beta, wg);

    const GreenFunctionPerformanceStats stats = get_green_function_performance_stats();
    require(stats.g_ns_value_requests == 1, "O contador de chamadas de G^NS deveria refletir o wrapper escalar.");
    require(stats.g_ns_dx_requests == 1, "O contador de dG^NS/dx' deveria refletir o wrapper escalar.");
    require(stats.g_ns_dy_requests == 1, "O contador de dG^NS/dy' deveria refletir o wrapper escalar.");
    require(stats.g_ns_bundle_evaluations == 5,
            "As avaliações em bloco deveriam contar chamadas internas e wrappers escalares.");
    require(stats.oscillatory_branch_evaluations == 3,
            "O ramo oscilatório deveria ser usado para os pontos com x != x'.");
    require(stats.transformed_branch_evaluations == 2,
            "O ramo transformado deveria ser usado para os pontos com x = x'.");
}

} // namespace

int main() {
    test_green_symmetry_and_decay();
    test_green_gradients_against_finite_difference();
    test_green_ns_gradients_against_finite_difference();
    test_ns_bundle_matches_scalar_wrappers_off_axis();
    test_ns_bundle_matches_scalar_wrappers_at_equal_abscissa();
    test_green_function_stats_track_requests_and_branches();
    std::cout << "Green function sanity checks passed." << std::endl;
    return 0;
}
