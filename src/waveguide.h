#pragma once

#include <cstddef>
#include <cmath>
#include <functional>
#include <vector>

constexpr double PI = 3.14159265358979323846;
constexpr double C0 = 299792458.0; // Velocidade da luz no vácuo

enum class ProfileType {
    HOMOGENEOUS,
    PARABOLIC_1D,
    CIRCULAR_2D
};

struct WaveguideParams {
    double n1;      // Índice de refração do superstrato (y < 0)
    double n3;      // Índice de refração do substrato
    double n2m;     // Índice de refração máximo no núcleo
    double a;       // Largura do guia (dimensão x)
    double b;       // Profundidade do guia (dimensão y)
    double lambda0; // Comprimento de onda no vácuo
    ProfileType profile_type;
};

struct Discretization {
    size_t Nx; // Número de pontos em x
    size_t Ny; // Número de pontos em y
};

struct Cell {
    double cx, cy; // Coordenadas do centro
    double dx, dy; // Dimensões da célula
    size_t id;     // Identificador único
};

class Waveguide {
public:
    Waveguide(const WaveguideParams& params, const Discretization& disc);

    const std::vector<Cell>& get_cells() const { return cells; }
    const WaveguideParams& get_params() const { return params; }
    double get_refractive_index(double x, double y) const;
    double get_k_squared(double x, double y) const;
    double get_k0() const { return 2.0 * PI / params.lambda0; }

private:
    WaveguideParams params;
    Discretization discretization;
    std::vector<Cell> cells;
    std::function<double(double, double, const WaveguideParams&)> n_profile_func;
};
