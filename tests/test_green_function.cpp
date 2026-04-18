#include <algorithm>
#include <cmath>
#include <complex>
#include <iostream>
#include <stdexcept>

#include "green_function.h"
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

} // namespace

int main() {
    test_green_symmetry_and_decay();
    test_green_gradients_against_finite_difference();
    std::cout << "Green function sanity checks passed." << std::endl;
    return 0;
}
