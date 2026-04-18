#pragma once

#include <complex>

#include "waveguide.h"

using Complex = std::complex<double>;

// Implementação atual:
// - suporta o regime guiado com beta > k1 e beta > k3;
// - usa diretamente a decomposição G = G^S + G^NS registrada em docs/02_teoria.md;
// - nesta etapa só está validada para pontos com y >= 0 e y' >= 0, que é o regime
//   hoje usado pelos casos internos e pelos testes de sanidade.
Complex calculate_G_S(double x, double y, double xp, double yp, double beta, const Waveguide& wg);
Complex calculate_G_NS(double x, double y, double xp, double yp, double beta, const Waveguide& wg);
Complex calculate_G(double x, double y, double xp, double yp, double beta, const Waveguide& wg);

Complex calculate_dG_S_dx_source(double x, double y, double xp, double yp, double beta, const Waveguide& wg);
Complex calculate_dG_S_dy_source(double x, double y, double xp, double yp, double beta, const Waveguide& wg);
Complex calculate_dG_NS_dx_source(double x, double y, double xp, double yp, double beta, const Waveguide& wg);
Complex calculate_dG_NS_dy_source(double x, double y, double xp, double yp, double beta, const Waveguide& wg);
