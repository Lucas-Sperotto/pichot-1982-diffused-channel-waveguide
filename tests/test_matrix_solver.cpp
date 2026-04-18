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
    AssemblyOptions options;
    options.include_boundary_distribution = false;

    build_matrix_A(A, beta, wg, options);

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

void test_boundary_segment_extraction_is_explicit() {
    const Waveguide homogeneous = make_small_homogeneous_waveguide();
    require(homogeneous.get_boundary_segments().size() == 10,
            "O caso homogêneo 3x2 deveria expor 10 segmentos de fronteira explícitos.");

    const Waveguide parabolic = make_small_parabolic_waveguide();
    require(parabolic.get_boundary_segments().size() == 9,
            "O caso parabólico 3x3 deveria expor 9 segmentos porque o topo é contínuo com o substrato.");

    bool found_bottom_jump = false;
    bool found_side_jump = false;
    for (const BoundarySegment& segment : parabolic.get_boundary_segments()) {
        if (segment.outward_normal.y < 0.0) {
            found_bottom_jump = true;
        }
        if (std::abs(segment.outward_normal.x) > 0.0) {
            found_side_jump = true;
        }
    }

    require(found_bottom_jump, "O perfil parabólico deveria expor segmentos de fronteira na interface inferior.");
    require(found_side_jump, "O perfil parabólico deveria expor segmentos de fronteira laterais.");
}

void test_parabolic_profile_introduces_regular_grad_coupling() {
    const Waveguide wg = make_small_parabolic_waveguide();
    const double beta = 0.5 * wg.get_k0() * (wg.get_params().n2m + wg.get_params().n3);
    const std::size_t matrix_size = 2 * wg.get_cells().size();
    ComplexMatrix A(matrix_size, matrix_size);
    AssemblyOptions options;
    options.include_boundary_distribution = false;

    build_matrix_A(A, beta, wg, options);

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

void test_boundary_distribution_changes_homogeneous_operator() {
    const Waveguide wg = make_small_homogeneous_waveguide();
    const double beta = 0.5 * wg.get_k0() * (wg.get_params().n2m + wg.get_params().n3);
    const std::size_t matrix_size = 2 * wg.get_cells().size();
    ComplexMatrix without_boundary(matrix_size, matrix_size);
    ComplexMatrix with_boundary(matrix_size, matrix_size);

    AssemblyOptions options_without_boundary;
    options_without_boundary.include_boundary_distribution = false;

    AssemblyOptions options_with_boundary;
    options_with_boundary.include_boundary_distribution = true;

    build_matrix_A(without_boundary, beta, wg, options_without_boundary);
    build_matrix_A(with_boundary, beta, wg, options_with_boundary);

    double max_difference = 0.0;
    double max_off_diagonal = 0.0;

    const std::size_t N = wg.get_cells().size();
    for (std::size_t i = 0; i < N; ++i) {
        for (std::size_t j = 0; j < N; ++j) {
            max_difference = std::max(max_difference, std::abs(with_boundary.at(i, j) - without_boundary.at(i, j)));
            max_difference = std::max(
                max_difference, std::abs(with_boundary.at(i + N, j + N) - without_boundary.at(i + N, j + N)));
            max_off_diagonal = std::max(max_off_diagonal, std::abs(with_boundary.at(i, j + N)));
            max_off_diagonal = std::max(max_off_diagonal, std::abs(with_boundary.at(i + N, j)));
        }
    }

    require(max_difference > 0.0,
            "O termo de fronteira deveria alterar a matriz mesmo no caso homogêneo em volume.");
    require(max_off_diagonal > 0.0,
            "O termo de fronteira deveria introduzir acoplamentos cruzados via normal da borda.");
}

void test_boundary_quadrature_models_are_distinct() {
    const Waveguide wg = make_small_homogeneous_waveguide();
    const double beta = 0.5 * wg.get_k0() * (wg.get_params().n2m + wg.get_params().n3);
    const std::size_t matrix_size = 2 * wg.get_cells().size();
    ComplexMatrix midpoint_matrix(matrix_size, matrix_size);
    ComplexMatrix gauss_matrix(matrix_size, matrix_size);

    AssemblyOptions midpoint_options;
    midpoint_options.include_boundary_distribution = true;
    midpoint_options.boundary_quadrature_model = BoundaryQuadratureModel::MIDPOINT;
    midpoint_options.boundary_subdivisions = 1;

    AssemblyOptions gauss_options;
    gauss_options.include_boundary_distribution = true;
    gauss_options.boundary_quadrature_model = BoundaryQuadratureModel::GAUSS2;
    gauss_options.boundary_subdivisions = 2;

    build_matrix_A(midpoint_matrix, beta, wg, midpoint_options);
    build_matrix_A(gauss_matrix, beta, wg, gauss_options);

    double max_difference = 0.0;
    for (std::size_t row = 0; row < matrix_size; ++row) {
        for (std::size_t col = 0; col < matrix_size; ++col) {
            max_difference = std::max(max_difference, std::abs(midpoint_matrix.at(row, col) - gauss_matrix.at(row, col)));
        }
    }

    require(max_difference > 0.0,
            "Modelos distintos de quadratura de fronteira deveriam produzir matrizes distintas.");
}

void test_beta_search_reduces_modal_residual() {
    const Waveguide wg = make_small_homogeneous_waveguide();
    const double beta_min = wg.get_k0() * wg.get_params().n3;
    const double beta_max = wg.get_k0() * wg.get_params().n2m;

    const double beta_candidate = find_beta_root(wg, beta_min, beta_max);
    const ModeSolution solution_candidate = solve_mode_at_beta(beta_candidate, wg);
    const ModeSolution solution_left = solve_mode_at_beta(beta_min + 1e-6 * wg.get_k0(), wg);
    const ModeSolution solution_right = solve_mode_at_beta(beta_max - 1e-6 * wg.get_k0(), wg);

    require(beta_candidate > beta_min, "beta candidato deveria ficar dentro do intervalo guiado.");
    require(beta_candidate < beta_max, "beta candidato deveria ficar dentro do intervalo guiado.");
    require(std::isfinite(solution_candidate.modal_residual), "O residual modal deveria ser finito.");
    require(std::isfinite(solution_candidate.determinant_magnitude),
            "O determinante deve continuar disponível como diagnóstico.");
    require(solution_candidate.modal_residual <= solution_left.modal_residual,
            "A busca modal deveria melhorar ou igualar o residual modal do extremo esquerdo.");
    require(solution_candidate.modal_residual <= solution_right.modal_residual,
            "A busca modal deveria melhorar ou igualar o residual modal do extremo direito.");
}

void test_mode_solution_returns_finite_coefficients_and_residual() {
    const Waveguide wg = make_small_parabolic_waveguide();
    const double beta_min = wg.get_k0() * wg.get_params().n3;
    const double beta_max = wg.get_k0() * wg.get_params().n2m;

    double beta_candidate = find_beta_root(wg, beta_min, beta_max);
    beta_candidate = refine_beta_with_modal_residual(beta_candidate, beta_min, beta_max, wg);
    const ModeSolution solution = solve_mode_at_beta(beta_candidate, wg);

    require(solution.beta > beta_min && solution.beta < beta_max,
            "A solução modal deveria manter beta no intervalo guiado.");
    require(std::isfinite(solution.determinant_magnitude),
            "O determinante associado à solução modal deveria ser finito.");
    require(std::isfinite(solution.modal_residual),
            "O residual modal deveria ser finito.");
    require(solution.modal_residual >= 0.0, "O residual modal não deveria ser negativo.");
    require(solution.coefficients.size() == 2 * wg.get_cells().size(),
            "O vetor modal deveria ter um coeficiente por componente e por célula.");

    double max_amplitude = 0.0;
    for (const Complex& coefficient : solution.coefficients) {
        max_amplitude = std::max(max_amplitude, std::abs(coefficient));
    }
    require(max_amplitude > 0.0, "A solução modal não deveria ser o vetor nulo.");
}

} // namespace

int main() {
    test_homogeneous_blocks_remain_decoupled();
    test_regular_gradient_field_matches_profile_expectation();
    test_boundary_segment_extraction_is_explicit();
    test_parabolic_profile_introduces_regular_grad_coupling();
    test_boundary_distribution_changes_homogeneous_operator();
    test_boundary_quadrature_models_are_distinct();
    test_beta_search_reduces_modal_residual();
    test_mode_solution_returns_finite_coefficients_and_residual();
    std::cout << "Matrix solver prototype checks passed." << std::endl;
    return 0;
}
