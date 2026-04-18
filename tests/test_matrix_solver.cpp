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

void test_scalar_blocks_are_decoupled() {
    const Waveguide wg = make_small_homogeneous_waveguide();
    const double beta = 0.5 * wg.get_k0() * (wg.get_params().n2m + wg.get_params().n3);
    const std::size_t matrix_size = 2 * wg.get_cells().size();
    ComplexMatrix A(matrix_size, matrix_size);

    build_matrix_A(A, beta, wg);

    const std::size_t N = wg.get_cells().size();
    for (std::size_t i = 0; i < N; ++i) {
        for (std::size_t j = 0; j < N; ++j) {
            require(std::abs(A.at(i, j + N)) < 1e-14, "O bloco A_xy deveria estar nulo nesta etapa.");
            require(std::abs(A.at(i + N, j)) < 1e-14, "O bloco A_yx deveria estar nulo nesta etapa.");
            require(std::abs(A.at(i, j) - A.at(i + N, j + N)) < 1e-10,
                    "Os blocos A_xx e A_yy deveriam coincidir no protótipo escalar.");
            require(std::isfinite(A.at(i, j).real()), "Os coeficientes da matriz deveriam ser finitos.");
        }
    }
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
    test_scalar_blocks_are_decoupled();
    test_beta_search_reduces_determinant_residual();
    std::cout << "Matrix solver prototype checks passed." << std::endl;
    return 0;
}
