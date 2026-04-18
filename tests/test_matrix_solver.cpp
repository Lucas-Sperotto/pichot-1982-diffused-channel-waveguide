#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>

#include "matrix_solver.h"
#include "waveguide.h"

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

Waveguide make_small_homogeneous_waveguide() {
    WaveguideParams params;
    params.n1 = 1.0;
    params.n3 = 1.47;
    params.n2m = 1.48;
    params.a = 2.0e-6;
    params.b = 1.0e-6;
    params.lambda0 = 0.85e-6;
    params.profile_type = ProfileType::HOMOGENEOUS;

    Discretization discretization;
    discretization.Nx = 3;
    discretization.Ny = 2;

    return Waveguide(params, discretization);
}

Waveguide make_small_parabolic_waveguide() {
    WaveguideParams params;
    params.n1 = 1.0;
    params.n3 = 1.47;
    params.n2m = 1.49;
    params.a = 2.22e-6;
    params.b = 1.11e-6;
    params.lambda0 = 0.85e-6;
    params.profile_type = ProfileType::PARABOLIC_1D;

    Discretization discretization;
    discretization.Nx = 3;
    discretization.Ny = 3;

    return Waveguide(params, discretization);
}

void test_homogeneous_blocks_remain_decoupled() {
    const Waveguide wg = make_small_homogeneous_waveguide();
    const double beta = 0.5 * wg.get_k0() * (wg.get_params().n2m + wg.get_params().n3);
    const std::size_t matrix_size = 2 * wg.get_cells().size();
    ComplexMatrix A(matrix_size, matrix_size);

    build_matrix_A(A, beta, wg);

    const std::size_t N = wg.get_cells().size();
    for (std::size_t i = 0; i < N; ++i) {
        for (std::size_t j = 0; j < N; ++j) {
            require(std::abs(A.at(i, j + N)) < 1e-14, "O bloco A_xy deveria ficar nulo no caso homogêneo.");
            require(std::abs(A.at(i + N, j)) < 1e-14, "O bloco A_yx deveria ficar nulo no caso homogêneo.");
            require(std::abs(A.at(i, j) - A.at(i + N, j + N)) < 1e-10,
                    "Os blocos A_xx e A_yy deveriam coincidir quando o gradiente regular é nulo.");
            require(std::isfinite(A.at(i, j).real()), "Os coeficientes da matriz deveriam ser finitos.");
        }
    }
}

void test_regular_gradient_field_matches_profile_expectation() {
    const Waveguide homogeneous = make_small_homogeneous_waveguide();
    const Vector2 homogeneous_regular = homogeneous.get_regular_epsilon_grad_inverse(0.0, 0.5e-6);
    require(std::abs(homogeneous_regular.x) < 1e-14 && std::abs(homogeneous_regular.y) < 1e-14,
            "O perfil homogêneo deveria ter campo regular ε grad(1/ε) nulo.");

    const Waveguide parabolic = make_small_parabolic_waveguide();
    const Vector2 parabolic_regular = parabolic.get_regular_epsilon_grad_inverse(0.0, 0.5 * parabolic.get_params().b);
    require(std::abs(parabolic_regular.x) < 1e-14,
            "O perfil parabólico 1-D não deveria gerar componente regular em x.");
    require(parabolic_regular.y > 0.0,
            "O perfil parabólico 1-D deveria gerar componente regular positiva em y no interior.");
}

void test_parabolic_profile_introduces_regular_grad_coupling() {
    const Waveguide wg = make_small_parabolic_waveguide();
    const double beta = 0.5 * wg.get_k0() * (wg.get_params().n2m + wg.get_params().n3);
    const std::size_t matrix_size = 2 * wg.get_cells().size();
    ComplexMatrix A(matrix_size, matrix_size);

    build_matrix_A(A, beta, wg);

    const std::size_t N = wg.get_cells().size();
    double max_xy = 0.0;
    double max_yx = 0.0;
    double max_diag_difference = 0.0;

    for (std::size_t i = 0; i < N; ++i) {
        for (std::size_t j = 0; j < N; ++j) {
            max_xy = std::max(max_xy, std::abs(A.at(i, j + N)));
            max_yx = std::max(max_yx, std::abs(A.at(i + N, j)));
            max_diag_difference =
                std::max(max_diag_difference, std::abs(A.at(i, j) - A.at(i + N, j + N)));
        }
    }

    require(max_xy > 0.0, "O termo regular com grad'G deveria introduzir acoplamento A_xy no caso parabólico.");
    require(max_diag_difference > 0.0,
            "O termo regular com grad'G deveria diferenciar A_xx e A_yy no caso parabólico.");
    require(max_yx < 1e-12,
            "No perfil parabólico 1-D, a parte regular não deveria introduzir A_yx porque o gradiente em x é nulo.");
}

void test_beta_search_reduces_determinant_residual() {
    const Waveguide wg = make_small_homogeneous_waveguide();
    const double beta_min = wg.get_k0() * wg.get_params().n3;
    const double beta_max = wg.get_k0() * wg.get_params().n2m;

    const double beta_candidate = find_beta_root(wg, beta_min, beta_max);
    const double residual_candidate = calculate_determinant_magnitude(beta_candidate, wg);
    const double residual_left = calculate_determinant_magnitude(beta_min + 1e-6 * wg.get_k0(), wg);
    const double residual_right = calculate_determinant_magnitude(beta_max - 1e-6 * wg.get_k0(), wg);

    require(beta_candidate > beta_min, "beta candidato deveria ficar dentro do intervalo guiado.");
    require(beta_candidate < beta_max, "beta candidato deveria ficar dentro do intervalo guiado.");
    require(std::isfinite(residual_candidate), "O residual do determinante deveria ser finito.");
    require(residual_candidate <= residual_left,
            "A busca modal deveria melhorar ou igualar o residual do extremo esquerdo.");
    require(residual_candidate <= residual_right,
            "A busca modal deveria melhorar ou igualar o residual do extremo direito.");
}

} // namespace

int main() {
    test_homogeneous_blocks_remain_decoupled();
    test_regular_gradient_field_matches_profile_expectation();
    test_parabolic_profile_introduces_regular_grad_coupling();
    test_beta_search_reduces_determinant_residual();
    std::cout << "Matrix solver prototype checks passed." << std::endl;
    return 0;
}
