#include "matrix_solver.h"

#include "green_function.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace {

constexpr double kPivotTolerance = 1e-18;
constexpr int kInitialSearchSamples = 7;
constexpr int kRefinementSamples = 5;
constexpr int kRefinementIterations = 4;
constexpr int kInverseIterations = 6;
constexpr int kModalRefinementSamples = 3;

constexpr double kGaussPoint = 0.5773502691896257;
constexpr std::size_t kSelfCellGaussSubdivisionsPerAxis = 4;

double get_background_k_squared(const Waveguide& wg) {
    const double k0 = wg.get_k0();
    return k0 * k0 * wg.get_params().n3 * wg.get_params().n3;
}

double get_beta_margin(const Waveguide& wg, double beta_min, double beta_max) {
    const double interval = beta_max - beta_min;
    return std::max(1e-9 * wg.get_k0(), 1e-4 * interval);
}

struct KernelAverages {
    Complex green_average{0.0, 0.0};
    Complex dG_dx_average{0.0, 0.0};
    Complex dG_dy_average{0.0, 0.0};
};

struct MatrixBlockContribution {
    Complex xx{0.0, 0.0};
    Complex xy{0.0, 0.0};
    Complex yx{0.0, 0.0};
    Complex yy{0.0, 0.0};
};

void accumulate_block(MatrixBlockContribution* total, const MatrixBlockContribution& increment) {
    total->xx += increment.xx;
    total->xy += increment.xy;
    total->yx += increment.yx;
    total->yy += increment.yy;
}

template <typename Integrand>
Complex average_over_cell_with_subcell_gauss2(const Cell& source, Integrand integrand) {
    const double sub_dx = source.dx / static_cast<double>(kSelfCellGaussSubdivisionsPerAxis);
    const double sub_dy = source.dy / static_cast<double>(kSelfCellGaussSubdivisionsPerAxis);
    const double half_sub_dx = 0.5 * sub_dx;
    const double half_sub_dy = 0.5 * sub_dy;
    Complex integral(0.0, 0.0);

    for (std::size_t ix = 0; ix < kSelfCellGaussSubdivisionsPerAxis; ++ix) {
        const double sub_center_x = source.cx - 0.5 * source.dx + (static_cast<double>(ix) + 0.5) * sub_dx;
        for (std::size_t iy = 0; iy < kSelfCellGaussSubdivisionsPerAxis; ++iy) {
            const double sub_center_y = source.cy - 0.5 * source.dy + (static_cast<double>(iy) + 0.5) * sub_dy;

            for (double sx : {-1.0, 1.0}) {
                for (double sy : {-1.0, 1.0}) {
                    const double x_source = sub_center_x + sx * kGaussPoint * half_sub_dx;
                    const double y_source = sub_center_y + sy * kGaussPoint * half_sub_dy;
                    integral += integrand(x_source, y_source) * (half_sub_dx * half_sub_dy);
                }
            }
        }
    }

    return integral / (source.dx * source.dy);
}

Complex evaluate_green_between_cells(const Cell& observation,
                                     const Cell& source,
                                     double beta,
                                     const Waveguide& wg) {
    if (observation.id != source.id) {
        return calculate_G(observation.cx, observation.cy, source.cx, source.cy, beta, wg);
    }

    // A auto-interação exige regularização da singularidade logarítmica de G^S.
    // Em vez da antiga média em quatro pontos, usamos uma média de célula por
    // subcélulas com quadratura de Gauss 2x2, que preserva a integral em toda a
    // área da função-base step sem avaliar o kernel exatamente no ponto singular.
    return average_over_cell_with_subcell_gauss2(source, [&](double x_source, double y_source) {
        return calculate_G(observation.cx, observation.cy, x_source, y_source, beta, wg);
    });
}

Complex evaluate_dG_dx_source_between_cells(const Cell& observation,
                                            const Cell& source,
                                            double beta,
                                            const Waveguide& wg) {
    if (observation.id != source.id) {
        return calculate_dG_S_dx_source(observation.cx, observation.cy, source.cx, source.cy, beta, wg) +
               calculate_dG_NS_dx_source(observation.cx, observation.cy, source.cx, source.cy, beta, wg);
    }

    // Para célula-fonte coincidente com a célula de observação, a média do termo
    // em x' se anula por simetria da função-base step em torno do centro da célula.
    return Complex(0.0, 0.0);
}

Complex evaluate_dG_dy_source_between_cells(const Cell& observation,
                                            const Cell& source,
                                            double beta,
                                            const Waveguide& wg) {
    if (observation.id != source.id) {
        return calculate_dG_S_dy_source(observation.cx, observation.cy, source.cx, source.cy, beta, wg) +
               calculate_dG_NS_dy_source(observation.cx, observation.cy, source.cx, source.cy, beta, wg);
    }

    // Na auto-interação, a parte singular G^S é ímpar em y' ao redor do centro da
    // célula e sua média se anula por simetria. A parte G^NS é regular e pode ser
    // integrada diretamente na célula pela mesma quadratura de subcélulas.
    return average_over_cell_with_subcell_gauss2(source, [&](double x_source, double y_source) {
        return calculate_dG_NS_dy_source(observation.cx, observation.cy, x_source, y_source, beta, wg);
    });
}

KernelAverages evaluate_kernel_averages_between_cells(const Cell& observation,
                                                      const Cell& source,
                                                      double beta,
                                                      const Waveguide& wg) {
    return KernelAverages{
        evaluate_green_between_cells(observation, source, beta, wg),
        evaluate_dG_dx_source_between_cells(observation, source, beta, wg),
        evaluate_dG_dy_source_between_cells(observation, source, beta, wg)};
}

MatrixBlockContribution make_identity_contribution(bool is_diagonal) {
    MatrixBlockContribution contribution;
    if (is_diagonal) {
        contribution.xx = Complex(1.0, 0.0);
        contribution.yy = Complex(1.0, 0.0);
    }
    return contribution;
}

MatrixBlockContribution make_scalar_contrast_contribution(const Cell& source,
                                                          const KernelAverages& kernels,
                                                          double background_k_squared,
                                                          const Waveguide& wg) {
    const double source_area = source.dx * source.dy;
    const double contrast_k_squared = wg.get_k_squared(source.cx, source.cy) - background_k_squared;
    const Complex scalar_kernel = contrast_k_squared * source_area * kernels.green_average;

    MatrixBlockContribution contribution;
    contribution.xx = -scalar_kernel;
    contribution.yy = -scalar_kernel;
    return contribution;
}

MatrixBlockContribution make_regular_gradient_contribution(const Cell& source,
                                                           const KernelAverages& kernels,
                                                           const Waveguide& wg) {
    const double source_area = source.dx * source.dy;
    const Vector2 epsilon_grad_inverse = wg.get_regular_epsilon_grad_inverse(source.cx, source.cy);

    MatrixBlockContribution contribution;
    contribution.xx = -source_area * epsilon_grad_inverse.x * kernels.dG_dx_average;
    contribution.xy = -source_area * epsilon_grad_inverse.y * kernels.dG_dx_average;
    contribution.yx = -source_area * epsilon_grad_inverse.x * kernels.dG_dy_average;
    contribution.yy = -source_area * epsilon_grad_inverse.y * kernels.dG_dy_average;
    return contribution;
}

MatrixBlockContribution make_boundary_segment_contribution(const Cell& observation,
                                                           const BoundarySegment& segment,
                                                           double beta,
                                                           const Waveguide& wg,
                                                           const AssemblyOptions& options) {
    const Vector2 tangent{-segment.outward_normal.y, segment.outward_normal.x};
    const std::size_t subdivisions = std::max<std::size_t>(1, options.boundary_subdivisions);
    const double subsegment_length = segment.length / static_cast<double>(subdivisions);
    Complex integrated_dG_dx_boundary(0.0, 0.0);
    Complex integrated_dG_dy_boundary(0.0, 0.0);

    for (std::size_t subdivision = 0; subdivision < subdivisions; ++subdivision) {
        const double sub_left = -0.5 * segment.length + subdivision * subsegment_length;
        const double sub_right = sub_left + subsegment_length;

        auto accumulate_boundary_sample = [&](double local_coordinate, double weight) {
            const double x_source = segment.x_midpoint + local_coordinate * tangent.x;
            const double y_source = segment.y_midpoint + local_coordinate * tangent.y;
            integrated_dG_dx_boundary +=
                weight * (calculate_dG_S_dx_source(observation.cx, observation.cy, x_source, y_source, beta, wg) +
                          calculate_dG_NS_dx_source(observation.cx, observation.cy, x_source, y_source, beta, wg));
            integrated_dG_dy_boundary +=
                weight * (calculate_dG_S_dy_source(observation.cx, observation.cy, x_source, y_source, beta, wg) +
                          calculate_dG_NS_dy_source(observation.cx, observation.cy, x_source, y_source, beta, wg));
        };

        if (options.boundary_quadrature_model == BoundaryQuadratureModel::MIDPOINT) {
            const double midpoint = 0.5 * (sub_left + sub_right);
            accumulate_boundary_sample(midpoint, subsegment_length);
        } else {
            const double midpoint = 0.5 * (sub_left + sub_right);
            const double half_length = 0.5 * (sub_right - sub_left);
            accumulate_boundary_sample(midpoint - kGaussPoint * half_length, half_length);
            accumulate_boundary_sample(midpoint + kGaussPoint * half_length, half_length);
        }
    }

    const Vector2 boundary_coefficient{
        segment.epsilon_jump_factor * segment.outward_normal.x,
        segment.epsilon_jump_factor * segment.outward_normal.y};

    MatrixBlockContribution contribution;
    contribution.xx = -boundary_coefficient.x * integrated_dG_dx_boundary;
    contribution.xy = -boundary_coefficient.y * integrated_dG_dx_boundary;
    contribution.yx = -boundary_coefficient.x * integrated_dG_dy_boundary;
    contribution.yy = -boundary_coefficient.y * integrated_dG_dy_boundary;
    return contribution;
}

MatrixBlockContribution make_boundary_distribution_contribution(const Cell& observation,
                                                                const Cell& source,
                                                                double beta,
                                                                const Waveguide& wg,
                                                                const AssemblyOptions& options) {
    MatrixBlockContribution contribution;
    for (const BoundarySegment& segment : wg.get_boundary_segments()) {
        if (segment.cell_id != source.id) {
            continue;
        }
        accumulate_block(&contribution, make_boundary_segment_contribution(observation, segment, beta, wg, options));
    }
    return contribution;
}

struct SearchSample {
    double beta = 0.0;
    double determinant_magnitude = std::numeric_limits<double>::infinity();
    double modal_residual = std::numeric_limits<double>::infinity();
};

SearchSample evaluate_sample(double beta, const Waveguide& wg, const AssemblyOptions& options) {
    SearchSample sample;
    sample.beta = beta;

    try {
        const ModeSolution solution = solve_mode_at_beta(beta, wg, options);
        if (std::isfinite(solution.determinant_magnitude)) {
            sample.determinant_magnitude = solution.determinant_magnitude;
        }
        if (std::isfinite(solution.modal_residual)) {
            sample.modal_residual = solution.modal_residual;
        }
    } catch (const std::exception&) {
        // Mantém infinito nos dois diagnósticos para marcar que a amostra falhou.
    }

    return sample;
}

bool is_better_search_sample(const SearchSample& lhs, const SearchSample& rhs) {
    const bool lhs_modal_finite = std::isfinite(lhs.modal_residual);
    const bool rhs_modal_finite = std::isfinite(rhs.modal_residual);
    if (lhs_modal_finite != rhs_modal_finite) {
        return lhs_modal_finite;
    }
    if (lhs.modal_residual != rhs.modal_residual) {
        return lhs.modal_residual < rhs.modal_residual;
    }

    const bool lhs_det_finite = std::isfinite(lhs.determinant_magnitude);
    const bool rhs_det_finite = std::isfinite(rhs.determinant_magnitude);
    if (lhs_det_finite != rhs_det_finite) {
        return lhs_det_finite;
    }
    if (lhs.determinant_magnitude != rhs.determinant_magnitude) {
        return lhs.determinant_magnitude < rhs.determinant_magnitude;
    }

    return lhs.beta < rhs.beta;
}

double squared_norm(const std::vector<Complex>& values) {
    double result = 0.0;
    for (const Complex& value : values) {
        result += std::norm(value);
    }
    return result;
}

double vector_norm(const std::vector<Complex>& values) {
    return std::sqrt(squared_norm(values));
}

std::vector<Complex> make_initial_mode_guess(std::size_t size) {
    std::vector<Complex> guess(size, Complex(0.0, 0.0));
    for (std::size_t i = 0; i < size; ++i) {
        guess[i] = Complex((i % 2 == 0) ? 1.0 : -1.0, 0.0);
    }
    return guess;
}

void normalize_vector(std::vector<Complex>* values) {
    const double norm = vector_norm(*values);
    if (norm <= kPivotTolerance) {
        throw std::runtime_error("Não foi possível normalizar o vetor modal.");
    }

    for (Complex& value : *values) {
        value /= norm;
    }
}

std::vector<Complex> multiply_matrix_vector(const ComplexMatrix& A, const std::vector<Complex>& x) {
    if (A.cols != x.size()) {
        throw std::runtime_error("Dimensões incompatíveis em multiply_matrix_vector.");
    }

    std::vector<Complex> result(A.rows, Complex(0.0, 0.0));
    for (std::size_t row = 0; row < A.rows; ++row) {
        Complex sum(0.0, 0.0);
        for (std::size_t col = 0; col < A.cols; ++col) {
            sum += A.at(row, col) * x[col];
        }
        result[row] = sum;
    }
    return result;
}

ComplexMatrix build_normal_matrix(const ComplexMatrix& A) {
    ComplexMatrix normal(A.cols, A.cols);
    for (std::size_t row = 0; row < A.rows; ++row) {
        for (std::size_t left = 0; left < A.cols; ++left) {
            const Complex conjugated = std::conj(A.at(row, left));
            for (std::size_t right = 0; right < A.cols; ++right) {
                normal.at(left, right) += conjugated * A.at(row, right);
            }
        }
    }
    return normal;
}

std::vector<Complex> solve_linear_system(ComplexMatrix A, std::vector<Complex> b) {
    if (A.rows != A.cols || A.rows != b.size()) {
        throw std::runtime_error("Dimensões incompatíveis em solve_linear_system.");
    }

    for (std::size_t pivot = 0; pivot < A.rows; ++pivot) {
        std::size_t pivot_row = pivot;
        double pivot_norm = std::abs(A.at(pivot, pivot));
        for (std::size_t row = pivot + 1; row < A.rows; ++row) {
            const double candidate = std::abs(A.at(row, pivot));
            if (candidate > pivot_norm) {
                pivot_norm = candidate;
                pivot_row = row;
            }
        }

        if (pivot_norm < kPivotTolerance) {
            throw std::runtime_error("Sistema linear singular ao estimar o vetor modal.");
        }

        if (pivot_row != pivot) {
            for (std::size_t col = 0; col < A.cols; ++col) {
                std::swap(A.at(pivot, col), A.at(pivot_row, col));
            }
            std::swap(b[pivot], b[pivot_row]);
        }

        const Complex pivot_value = A.at(pivot, pivot);
        for (std::size_t row = pivot + 1; row < A.rows; ++row) {
            const Complex factor = A.at(row, pivot) / pivot_value;
            if (std::abs(factor) < kPivotTolerance) {
                continue;
            }
            for (std::size_t col = pivot; col < A.cols; ++col) {
                A.at(row, col) -= factor * A.at(pivot, col);
            }
            b[row] -= factor * b[pivot];
        }
    }

    std::vector<Complex> x(A.rows, Complex(0.0, 0.0));
    for (std::size_t offset = 0; offset < A.rows; ++offset) {
        const std::size_t row = A.rows - 1 - offset;
        Complex rhs = b[row];
        for (std::size_t col = row + 1; col < A.cols; ++col) {
            rhs -= A.at(row, col) * x[col];
        }
        x[row] = rhs / A.at(row, row);
    }
    return x;
}

std::vector<Complex> estimate_smallest_singular_vector(const ComplexMatrix& A) {
    if (A.cols == 0) {
        return {};
    }

    ComplexMatrix normal = build_normal_matrix(A);
    double diagonal_scale = 0.0;
    for (std::size_t i = 0; i < normal.rows; ++i) {
        diagonal_scale += std::abs(normal.at(i, i));
    }
    diagonal_scale = std::max(diagonal_scale / static_cast<double>(normal.rows), 1.0);
    const double regularization = std::max(1e-14, 1e-12 * diagonal_scale);
    for (std::size_t i = 0; i < normal.rows; ++i) {
        normal.at(i, i) += regularization;
    }

    std::vector<Complex> iterate = make_initial_mode_guess(A.cols);
    normalize_vector(&iterate);

    for (int iteration = 0; iteration < kInverseIterations; ++iteration) {
        std::vector<Complex> next = solve_linear_system(normal, iterate);
        normalize_vector(&next);
        iterate.swap(next);
    }

    return iterate;
}

double calculate_modal_residual_from_solution(const ComplexMatrix& A, const std::vector<Complex>& coefficients) {
    const std::vector<Complex> residual_vector = multiply_matrix_vector(A, coefficients);
    const double denominator = std::max(vector_norm(coefficients), kPivotTolerance);
    return vector_norm(residual_vector) / denominator;
}

SearchSample best_sample_in_grid(double left,
                                 double right,
                                 int sample_count,
                                 const Waveguide& wg,
                                 const AssemblyOptions& options,
                                 double* next_left,
                                 double* next_right) {
    SearchSample best;
    best.beta = left;
    std::vector<SearchSample> samples;
    samples.reserve(sample_count);

    for (int i = 0; i < sample_count; ++i) {
        const double alpha = sample_count == 1 ? 0.0 : static_cast<double>(i) / (sample_count - 1);
        samples.push_back(evaluate_sample(left + alpha * (right - left), wg, options));
    }

    auto best_it = std::min_element(samples.begin(), samples.end(), [](const SearchSample& lhs, const SearchSample& rhs) {
        return is_better_search_sample(lhs, rhs);
    });
    best = *best_it;

    const std::size_t best_index = static_cast<std::size_t>(best_it - samples.begin());
    const std::size_t left_index = best_index == 0 ? 0 : best_index - 1;
    const std::size_t right_index = std::min(best_index + 1, samples.size() - 1);
    *next_left = samples[left_index].beta;
    *next_right = samples[right_index].beta;
    return best;
}

} // namespace

const char* boundary_quadrature_model_to_cstr(BoundaryQuadratureModel model) {
    switch (model) {
        case BoundaryQuadratureModel::MIDPOINT:
            return "midpoint";
        case BoundaryQuadratureModel::GAUSS2:
            return "gauss2";
    }

    throw std::runtime_error("BoundaryQuadratureModel inválido.");
}

ComplexMatrix::ComplexMatrix(std::size_t rows_in, std::size_t cols_in)
    : rows(rows_in), cols(cols_in), data(rows_in * cols_in, Complex(0.0, 0.0)) {}

Complex& ComplexMatrix::at(std::size_t row, std::size_t col) {
    return data.at(row * cols + col);
}

const Complex& ComplexMatrix::at(std::size_t row, std::size_t col) const {
    return data.at(row * cols + col);
}

void build_matrix_A(ComplexMatrix& A, double beta, const Waveguide& wg) {
    build_matrix_A(A, beta, wg, AssemblyOptions{});
}

void build_matrix_A(ComplexMatrix& A, double beta, const Waveguide& wg, const AssemblyOptions& options) {
    const auto& cells = wg.get_cells();
    const std::size_t N = cells.size();
    const double background_k_squared = get_background_k_squared(wg);

    if (A.rows != 2 * N || A.cols != 2 * N) {
        throw std::runtime_error("Dimensões da matriz A incorretas.");
    }

    std::fill(A.data.begin(), A.data.end(), Complex(0.0, 0.0));

    for (std::size_t i = 0; i < N; ++i) {
        const Cell& observation = cells[i];
        for (std::size_t j = 0; j < N; ++j) {
            const Cell& source = cells[j];
            MatrixBlockContribution total_contribution = make_identity_contribution(i == j);

            if (options.include_scalar_contrast || options.include_regular_gradient) {
                const KernelAverages kernels =
                    evaluate_kernel_averages_between_cells(observation, source, beta, wg);

                if (options.include_scalar_contrast) {
                    accumulate_block(
                        &total_contribution,
                        make_scalar_contrast_contribution(source, kernels, background_k_squared, wg));
                }

                if (options.include_regular_gradient) {
                    accumulate_block(&total_contribution, make_regular_gradient_contribution(source, kernels, wg));
                }
            }

            if (options.include_boundary_distribution) {
                accumulate_block(
                    &total_contribution,
                    make_boundary_distribution_contribution(observation, source, beta, wg, options));
            }

            A.at(i, j) = total_contribution.xx;
            A.at(i, j + N) = total_contribution.xy;
            A.at(i + N, j) = total_contribution.yx;
            A.at(i + N, j + N) = total_contribution.yy;
        }
    }
}

Complex calculate_determinant(const ComplexMatrix& A) {
    if (A.rows != A.cols) {
        throw std::runtime_error("O determinante exige uma matriz quadrada.");
    }

    ComplexMatrix triangular = A;
    Complex det(1.0, 0.0);
    int swap_count = 0;

    for (std::size_t pivot = 0; pivot < triangular.rows; ++pivot) {
        std::size_t pivot_row = pivot;
        double pivot_norm = std::abs(triangular.at(pivot, pivot));

        for (std::size_t row = pivot + 1; row < triangular.rows; ++row) {
            const double candidate_norm = std::abs(triangular.at(row, pivot));
            if (candidate_norm > pivot_norm) {
                pivot_norm = candidate_norm;
                pivot_row = row;
            }
        }

        if (pivot_norm < kPivotTolerance) {
            return Complex(0.0, 0.0);
        }

        if (pivot_row != pivot) {
            for (std::size_t col = 0; col < triangular.cols; ++col) {
                std::swap(triangular.at(pivot, col), triangular.at(pivot_row, col));
            }
            ++swap_count;
        }

        const Complex pivot_value = triangular.at(pivot, pivot);
        det *= pivot_value;

        for (std::size_t row = pivot + 1; row < triangular.rows; ++row) {
            const Complex factor = triangular.at(row, pivot) / pivot_value;
            if (std::abs(factor) < kPivotTolerance) {
                continue;
            }

            for (std::size_t col = pivot; col < triangular.cols; ++col) {
                triangular.at(row, col) -= factor * triangular.at(pivot, col);
            }
        }
    }

    if (swap_count % 2 != 0) {
        det = -det;
    }

    return det;
}

double calculate_determinant_magnitude(double beta, const Waveguide& wg) {
    return calculate_determinant_magnitude(beta, wg, AssemblyOptions{});
}

double calculate_determinant_magnitude(double beta, const Waveguide& wg, const AssemblyOptions& options) {
    try {
        const std::size_t matrix_size = 2 * wg.get_cells().size();
        ComplexMatrix A(matrix_size, matrix_size);
        build_matrix_A(A, beta, wg, options);
        const double residual = std::abs(calculate_determinant(A));
        return std::isfinite(residual) ? residual : std::numeric_limits<double>::infinity();
    } catch (const std::exception&) {
        return std::numeric_limits<double>::infinity();
    }
}

ModeSolution solve_mode_at_beta(double beta, const Waveguide& wg) {
    return solve_mode_at_beta(beta, wg, AssemblyOptions{});
}

ModeSolution solve_mode_at_beta(double beta, const Waveguide& wg, const AssemblyOptions& options) {
    const std::size_t matrix_size = 2 * wg.get_cells().size();
    ComplexMatrix A(matrix_size, matrix_size);
    build_matrix_A(A, beta, wg, options);

    ModeSolution solution;
    solution.beta = beta;
    solution.determinant_magnitude = std::abs(calculate_determinant(A));
    solution.coefficients = estimate_smallest_singular_vector(A);
    solution.modal_residual = calculate_modal_residual_from_solution(A, solution.coefficients);
    return solution;
}

double calculate_modal_residual(double beta, const Waveguide& wg) {
    return calculate_modal_residual(beta, wg, AssemblyOptions{});
}

double calculate_modal_residual(double beta, const Waveguide& wg, const AssemblyOptions& options) {
    try {
        const ModeSolution solution = solve_mode_at_beta(beta, wg, options);
        return std::isfinite(solution.modal_residual) ? solution.modal_residual
                                                      : std::numeric_limits<double>::infinity();
    } catch (const std::exception&) {
        return std::numeric_limits<double>::infinity();
    }
}

double refine_beta_with_modal_residual(double beta_initial,
                                       double beta_min,
                                       double beta_max,
                                       const Waveguide& wg) {
    return refine_beta_with_modal_residual(beta_initial, beta_min, beta_max, wg, AssemblyOptions{});
}

double refine_beta_with_modal_residual(double beta_initial,
                                       double beta_min,
                                       double beta_max,
                                       const Waveguide& wg,
                                       const AssemblyOptions& options) {
    const double interval = beta_max - beta_min;
    const double margin = std::max(get_beta_margin(wg, beta_min, beta_max), 0.01 * interval);
    const double left = std::max(beta_min + kPivotTolerance, beta_initial - margin);
    const double right = std::min(beta_max - kPivotTolerance, beta_initial + margin);
    if (!(left < right)) {
        return beta_initial;
    }

    SearchSample best_sample = evaluate_sample(beta_initial, wg, options);
    for (int index = 0; index < kModalRefinementSamples; ++index) {
        const double alpha = kModalRefinementSamples == 1
                                 ? 0.0
                                 : static_cast<double>(index) / (kModalRefinementSamples - 1);
        const double beta = left + alpha * (right - left);
        const SearchSample candidate = evaluate_sample(beta, wg, options);
        if (is_better_search_sample(candidate, best_sample)) {
            best_sample = candidate;
        }
    }
    return best_sample.beta;
}

double find_beta_root(const Waveguide& wg, double beta_min, double beta_max) {
    return find_beta_root(wg, beta_min, beta_max, AssemblyOptions{});
}

double find_beta_root(const Waveguide& wg,
                      double beta_min,
                      double beta_max,
                      const AssemblyOptions& options) {
    const double margin = get_beta_margin(wg, beta_min, beta_max);
    double left = beta_min + margin;
    double right = beta_max - margin;

    if (!(left < right)) {
        return 0.5 * (beta_min + beta_max);
    }

    double next_left = left;
    double next_right = right;
    SearchSample best =
        best_sample_in_grid(left, right, kInitialSearchSamples, wg, options, &next_left, &next_right);
    left = next_left;
    right = next_right;

    for (int iteration = 0; iteration < kRefinementIterations; ++iteration) {
        SearchSample refined = best_sample_in_grid(
            left, right, kRefinementSamples, wg, options, &next_left, &next_right);
        if (is_better_search_sample(refined, best)) {
            best = refined;
        }
        left = next_left;
        right = next_right;
        if (!(left < right)) {
            break;
        }
    }

    return best.beta;
}
