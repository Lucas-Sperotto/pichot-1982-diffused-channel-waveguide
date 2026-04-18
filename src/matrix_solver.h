#pragma once

#include <complex>
#include <vector>

#include "waveguide.h"

using Complex = std::complex<double>;

struct ComplexMatrix {
    std::size_t rows;
    std::size_t cols;
    std::vector<Complex> data;

    ComplexMatrix(std::size_t rows, std::size_t cols);

    Complex& at(std::size_t row, std::size_t col);
    const Complex& at(std::size_t row, std::size_t col) const;
};

void build_matrix_A(ComplexMatrix& A, double beta, const Waveguide& wg);
Complex calculate_determinant(const ComplexMatrix& A);
double find_beta_root(const Waveguide& wg, double beta_min, double beta_max);
