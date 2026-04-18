#include "matrix_solver.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

ComplexMatrix::ComplexMatrix(std::size_t rows_in, std::size_t cols_in)
    : rows(rows_in), cols(cols_in), data(rows_in * cols_in, Complex(0.0, 0.0)) {}

Complex& ComplexMatrix::at(std::size_t row, std::size_t col) {
    return data.at(row * cols + col);
}

const Complex& ComplexMatrix::at(std::size_t row, std::size_t col) const {
    return data.at(row * cols + col);
}

void build_matrix_A(ComplexMatrix& A, double beta, const Waveguide& wg) {
    (void)beta;

    const auto& cells = wg.get_cells();
    const std::size_t N = cells.size();

    if (A.rows != 2 * N || A.cols != 2 * N) {
        throw std::runtime_error("Dimensões da matriz A incorretas.");
    }

    std::fill(A.data.begin(), A.data.end(), Complex(0.0, 0.0));

    for (std::size_t i = 0; i < N; ++i) {
        for (std::size_t j = 0; j < N; ++j) {
            Complex A_xx(0.0, 0.0);
            Complex A_xy(0.0, 0.0);
            Complex A_yx(0.0, 0.0);
            Complex A_yy(0.0, 0.0);

            if (i == j) {
                // TODO: Substituir a identidade pelo operador integral discretizado do artigo.
                A_xx = Complex(1.0, 0.0);
                A_yy = Complex(1.0, 0.0);
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

        if (pivot_norm < 1e-18) {
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
            if (std::abs(factor) < 1e-18) {
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


double find_beta_root(const Waveguide& wg, double beta_min, double beta_max) {
    const std::size_t matrix_size = 2 * wg.get_cells().size();
    ComplexMatrix A(matrix_size, matrix_size);
    const double beta_mid = 0.5 * (beta_min + beta_max);

    build_matrix_A(A, beta_mid, wg);
    (void)calculate_determinant(A);

    // TODO: Implementar a busca de raízes de det(A) = 0 conforme o método do artigo.
    return beta_mid;
}
