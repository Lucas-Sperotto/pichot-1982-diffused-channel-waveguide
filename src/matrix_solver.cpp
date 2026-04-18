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

double get_background_k_squared(const Waveguide& wg) {
    const double k0 = wg.get_k0();
    return k0 * k0 * wg.get_params().n3 * wg.get_params().n3;
}

double get_beta_margin(const Waveguide& wg, double beta_min, double beta_max) {
    const double interval = beta_max - beta_min;
    return std::max(1e-9 * wg.get_k0(), 1e-4 * interval);
}

Complex evaluate_green_between_cells(const Cell& observation,
                                     const Cell& source,
                                     double beta,
                                     const Waveguide& wg) {
    if (observation.id != source.id) {
        return calculate_G(observation.cx, observation.cy, source.cx, source.cy, beta, wg);
    }

    // A auto-interação exige regularização da singularidade logarítmica de G^S.
    // Nesta etapa usamos uma média em quatro subpontos dentro da célula fonte para
    // manter o termo auditável e finito sem alegar uma quadratura exata do artigo.
    const double x_offset = 0.25 * source.dx;
    const double y_offset = 0.25 * source.dy;
    Complex average(0.0, 0.0);

    for (double sx : {-1.0, 1.0}) {
        for (double sy : {-1.0, 1.0}) {
            average += calculate_G(observation.cx,
                                   observation.cy,
                                   source.cx + sx * x_offset,
                                   source.cy + sy * y_offset,
                                   beta,
                                   wg);
        }
    }

    return average / 4.0;
}

Complex evaluate_dG_dx_source_between_cells(const Cell& observation,
                                            const Cell& source,
                                            double beta,
                                            const Waveguide& wg) {
    if (observation.id != source.id) {
        return calculate_dG_S_dx_source(observation.cx, observation.cy, source.cx, source.cy, beta, wg) +
               calculate_dG_NS_dx_source(observation.cx, observation.cy, source.cx, source.cy, beta, wg);
    }

    const double x_offset = 0.25 * source.dx;
    const double y_offset = 0.25 * source.dy;
    Complex average(0.0, 0.0);

    for (double sx : {-1.0, 1.0}) {
        for (double sy : {-1.0, 1.0}) {
            average += calculate_dG_S_dx_source(observation.cx,
                                                observation.cy,
                                                source.cx + sx * x_offset,
                                                source.cy + sy * y_offset,
                                                beta,
                                                wg) +
                       calculate_dG_NS_dx_source(observation.cx,
                                                 observation.cy,
                                                 source.cx + sx * x_offset,
                                                 source.cy + sy * y_offset,
                                                 beta,
                                                 wg);
        }
    }

    return average / 4.0;
}

Complex evaluate_dG_dy_source_between_cells(const Cell& observation,
                                            const Cell& source,
                                            double beta,
                                            const Waveguide& wg) {
    if (observation.id != source.id) {
        return calculate_dG_S_dy_source(observation.cx, observation.cy, source.cx, source.cy, beta, wg) +
               calculate_dG_NS_dy_source(observation.cx, observation.cy, source.cx, source.cy, beta, wg);
    }

    const double x_offset = 0.25 * source.dx;
    const double y_offset = 0.25 * source.dy;
    Complex average(0.0, 0.0);

    for (double sx : {-1.0, 1.0}) {
        for (double sy : {-1.0, 1.0}) {
            average += calculate_dG_S_dy_source(observation.cx,
                                                observation.cy,
                                                source.cx + sx * x_offset,
                                                source.cy + sy * y_offset,
                                                beta,
                                                wg) +
                       calculate_dG_NS_dy_source(observation.cx,
                                                 observation.cy,
                                                 source.cx + sx * x_offset,
                                                 source.cy + sy * y_offset,
                                                 beta,
                                                 wg);
        }
    }

    return average / 4.0;
}

struct SearchSample {
    double beta;
    double residual;
};

SearchSample evaluate_sample(double beta, const Waveguide& wg, const AssemblyOptions& options) {
    return SearchSample{beta, calculate_determinant_magnitude(beta, wg, options)};
}

SearchSample best_sample_in_grid(double left,
                                 double right,
                                 int sample_count,
                                 const Waveguide& wg,
                                 const AssemblyOptions& options,
                                 double* next_left,
                                 double* next_right) {
    SearchSample best{left, std::numeric_limits<double>::infinity()};
    std::vector<SearchSample> samples;
    samples.reserve(sample_count);

    for (int i = 0; i < sample_count; ++i) {
        const double alpha = sample_count == 1 ? 0.0 : static_cast<double>(i) / (sample_count - 1);
        samples.push_back(evaluate_sample(left + alpha * (right - left), wg, options));
    }

    auto best_it = std::min_element(
        samples.begin(), samples.end(), [](const SearchSample& lhs, const SearchSample& rhs) {
            return lhs.residual < rhs.residual;
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
            Complex A_xx(0.0, 0.0);
            Complex A_xy(0.0, 0.0);
            Complex A_yx(0.0, 0.0);
            Complex A_yy(0.0, 0.0);

            if (i == j) {
                A_xx = Complex(1.0, 0.0);
                A_yy = Complex(1.0, 0.0);
            }

            const double source_area = source.dx * source.dy;

            if (options.include_scalar_contrast || options.include_regular_gradient) {
                const Complex green_average = evaluate_green_between_cells(observation, source, beta, wg);
                const Complex dG_dx_average =
                    evaluate_dG_dx_source_between_cells(observation, source, beta, wg);
                const Complex dG_dy_average =
                    evaluate_dG_dy_source_between_cells(observation, source, beta, wg);

                if (options.include_scalar_contrast) {
                    const double contrast_k_squared =
                        wg.get_k_squared(source.cx, source.cy) - background_k_squared;
                    const Complex scalar_kernel = contrast_k_squared * source_area * green_average;
                    A_xx -= scalar_kernel;
                    A_yy -= scalar_kernel;
                }

                if (options.include_regular_gradient) {
                    const Vector2 epsilon_grad_inverse =
                        wg.get_regular_epsilon_grad_inverse(source.cx, source.cy);
                    const Complex regular_kernel_xx = source_area * epsilon_grad_inverse.x * dG_dx_average;
                    const Complex regular_kernel_xy = source_area * epsilon_grad_inverse.y * dG_dx_average;
                    const Complex regular_kernel_yx = source_area * epsilon_grad_inverse.x * dG_dy_average;
                    const Complex regular_kernel_yy = source_area * epsilon_grad_inverse.y * dG_dy_average;

                    A_xx -= regular_kernel_xx;
                    A_xy -= regular_kernel_xy;
                    A_yx -= regular_kernel_yx;
                    A_yy -= regular_kernel_yy;
                }
            }

            if (options.include_boundary_distribution) {
                for (const BoundarySegment& segment : wg.get_boundary_segments()) {
                    if (segment.cell_id != source.id) {
                        continue;
                    }

                    const Complex dG_dx_boundary =
                        calculate_dG_S_dx_source(observation.cx,
                                                 observation.cy,
                                                 segment.x_midpoint,
                                                 segment.y_midpoint,
                                                 beta,
                                                 wg) +
                        calculate_dG_NS_dx_source(observation.cx,
                                                  observation.cy,
                                                  segment.x_midpoint,
                                                  segment.y_midpoint,
                                                  beta,
                                                  wg);
                    const Complex dG_dy_boundary =
                        calculate_dG_S_dy_source(observation.cx,
                                                 observation.cy,
                                                 segment.x_midpoint,
                                                 segment.y_midpoint,
                                                 beta,
                                                 wg) +
                        calculate_dG_NS_dy_source(observation.cx,
                                                  observation.cy,
                                                  segment.x_midpoint,
                                                  segment.y_midpoint,
                                                  beta,
                                                  wg);
                    const Vector2 boundary_coefficient{
                        segment.epsilon_jump_factor * segment.outward_normal.x,
                        segment.epsilon_jump_factor * segment.outward_normal.y};
                    const Complex boundary_kernel_xx =
                        segment.length * boundary_coefficient.x * dG_dx_boundary;
                    const Complex boundary_kernel_xy =
                        segment.length * boundary_coefficient.y * dG_dx_boundary;
                    const Complex boundary_kernel_yx =
                        segment.length * boundary_coefficient.x * dG_dy_boundary;
                    const Complex boundary_kernel_yy =
                        segment.length * boundary_coefficient.y * dG_dy_boundary;

                    A_xx -= boundary_kernel_xx;
                    A_xy -= boundary_kernel_xy;
                    A_yx -= boundary_kernel_yx;
                    A_yy -= boundary_kernel_yy;
                }
            }

            A.at(i, j) = A_xx;
            A.at(i, j + N) = A_xy;
            A.at(i + N, j) = A_yx;
            A.at(i + N, j + N) = A_yy;
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
        if (refined.residual < best.residual) {
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
