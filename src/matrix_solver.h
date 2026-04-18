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

// Etapa atual:
// - operador escalar protótipo baseado apenas no termo (k^2 - k3^2) G da equação integral;
// - blocos Ex e Ey desacoplados e idênticos;
// - busca modal por mínimo de |det(A)|, ainda não pela formulação vetorial completa do artigo.
void build_matrix_A(ComplexMatrix& A, double beta, const Waveguide& wg);
Complex calculate_determinant(const ComplexMatrix& A);
double calculate_determinant_magnitude(double beta, const Waveguide& wg);
double find_beta_root(const Waveguide& wg, double beta_min, double beta_max);
