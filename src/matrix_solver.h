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

enum class BoundaryQuadratureModel {
    MIDPOINT,
    GAUSS2
};

struct AssemblyOptions {
    bool include_scalar_contrast = true;
    bool include_regular_gradient = true;
    bool include_boundary_distribution = true;
    BoundaryQuadratureModel boundary_quadrature_model = BoundaryQuadratureModel::GAUSS2;
    std::size_t boundary_subdivisions = 2;
};

struct ModeSolution {
    double beta = 0.0;
    double determinant_magnitude = 0.0;
    double modal_residual = 0.0;
    std::vector<Complex> coefficients;
};

// Etapa atual:
// - termo escalar (k^2 - k3^2) G;
// - parte volumétrica regular de eps grad(1/eps) multiplicando grad'G;
// - termo de fronteira isolado como soma explícita sobre segmentos da borda da malha;
// - busca modal por mínimo de |det(A)|, ainda não pela formulação vetorial completa do artigo.
void build_matrix_A(ComplexMatrix& A, double beta, const Waveguide& wg);
void build_matrix_A(ComplexMatrix& A, double beta, const Waveguide& wg, const AssemblyOptions& options);
Complex calculate_determinant(const ComplexMatrix& A);
double calculate_determinant_magnitude(double beta, const Waveguide& wg);
double calculate_determinant_magnitude(double beta, const Waveguide& wg, const AssemblyOptions& options);
double calculate_modal_residual(double beta, const Waveguide& wg);
double calculate_modal_residual(double beta, const Waveguide& wg, const AssemblyOptions& options);
ModeSolution solve_mode_at_beta(double beta, const Waveguide& wg);
ModeSolution solve_mode_at_beta(double beta, const Waveguide& wg, const AssemblyOptions& options);
double refine_beta_with_modal_residual(double beta_initial,
                                       double beta_min,
                                       double beta_max,
                                       const Waveguide& wg);
double refine_beta_with_modal_residual(double beta_initial,
                                       double beta_min,
                                       double beta_max,
                                       const Waveguide& wg,
                                       const AssemblyOptions& options);
double find_beta_root(const Waveguide& wg, double beta_min, double beta_max);
double find_beta_root(const Waveguide& wg, double beta_min, double beta_max, const AssemblyOptions& options);

const char* boundary_quadrature_model_to_cstr(BoundaryQuadratureModel model);
