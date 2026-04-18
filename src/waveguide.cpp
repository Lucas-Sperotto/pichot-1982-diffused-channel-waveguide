#include "waveguide.h"
#include <cmath>
#include <stdexcept>

namespace {

double background_index(double y, const WaveguideParams& p) {
    return y < 0.0 ? p.n1 : p.n3;
}

bool inside_core_rectangle(double x, double y, const WaveguideParams& p) {
    return x >= -p.a / 2.0 && x <= p.a / 2.0 && y >= 0.0 && y <= p.b;
}

Vector2 zero_vector() {
    return Vector2{0.0, 0.0};
}

} // namespace

double n_homogeneous(double x, double y, const WaveguideParams& p) {
    if (!inside_core_rectangle(x, y, p)) {
        return background_index(y, p);
    }

    return p.n2m;
}

Vector2 grad_n_homogeneous(double x, double y, const WaveguideParams& p) {
    (void)x;
    (void)y;
    (void)p;
    return zero_vector();
}

double n_parabolic_1d(double x, double y, const WaveguideParams& p) {
    if (!inside_core_rectangle(x, y, p)) {
        return background_index(y, p);
    }

    if (y > 0.0 && y < p.b) {
        return p.n3 + (p.n2m - p.n3) * (p.b * p.b - y * y) / (p.b * p.b);
    }

    return background_index(y, p);
}

Vector2 grad_n_parabolic_1d(double x, double y, const WaveguideParams& p) {
    (void)x;

    if (!inside_core_rectangle(x, y, p) || y <= 0.0 || y >= p.b) {
        return zero_vector();
    }

    const double delta_n = p.n2m - p.n3;
    return Vector2{0.0, -2.0 * delta_n * y / (p.b * p.b)};
}

double n_circular_2d(double x, double y, const WaveguideParams& p) {
    if (!inside_core_rectangle(x, y, p)) {
        return background_index(y, p);
    }

    double L2;
    if (y > x) {
        L2 = p.b * p.b + x * x;
    } else {
        L2 = (p.a / 2.0) * (p.a / 2.0) + y * y;
    }
    if (L2 > 1e-9) { // Evita divisão por zero
        return p.n3 + (p.n2m - p.n3) / L2 * (L2 - x * x - y * y);
    }

    return p.n2m;
}

Vector2 grad_n_circular_2d(double x, double y, const WaveguideParams& p) {
    if (!inside_core_rectangle(x, y, p)) {
        return zero_vector();
    }

    const double delta_n = p.n2m - p.n3;
    const double half_a = p.a / 2.0;

    if (y > x) {
        const double denominator = p.b * p.b + x * x;
        const double numerator = p.b * p.b - y * y;
        return Vector2{
            -2.0 * delta_n * x * numerator / (denominator * denominator),
            -2.0 * delta_n * y / denominator};
    }

    const double denominator = half_a * half_a + y * y;
    const double numerator = half_a * half_a - x * x;
    return Vector2{
        -2.0 * delta_n * x / denominator,
        -2.0 * delta_n * y * numerator / (denominator * denominator)};
}


Waveguide::Waveguide(const WaveguideParams& params, const Discretization& disc)
    : params(params), discretization(disc) {

    switch (params.profile_type) {
        case ProfileType::HOMOGENEOUS:
            n_profile_func = n_homogeneous;
            n_gradient_func = grad_n_homogeneous;
            break;
        case ProfileType::PARABOLIC_1D:
            n_profile_func = n_parabolic_1d;
            n_gradient_func = grad_n_parabolic_1d;
            break;
        case ProfileType::CIRCULAR_2D:
            n_profile_func = n_circular_2d;
            n_gradient_func = grad_n_circular_2d;
            break;
        default: throw std::runtime_error("Tipo de perfil de índice desconhecido.");
    }

    // Cria a malha de células (região do núcleo D2)
    double dx = params.a / discretization.Nx;
    double dy = params.b / discretization.Ny;
    size_t current_id = 0;
    for (size_t i = 0; i < discretization.Nx; ++i) {
        for (size_t j = 0; j < discretization.Ny; ++j) {
            Cell cell;
            cell.cx = -params.a / 2.0 + (i + 0.5) * dx;
            cell.cy = (j + 0.5) * dy; // O guia está em y > 0
            cell.dx = dx;
            cell.dy = dy;
            cell.id = current_id++;
            cells.push_back(cell);
        }
    }
}

double Waveguide::get_refractive_index(double x, double y) const {
    return n_profile_func(x, y, params);
}

Vector2 Waveguide::get_regular_refractive_index_gradient(double x, double y) const {
    return n_gradient_func(x, y, params);
}

Vector2 Waveguide::get_regular_epsilon_grad_inverse(double x, double y) const {
    const double refractive_index = get_refractive_index(x, y);
    if (refractive_index <= 0.0) {
        throw std::runtime_error("Índice de refração não positivo ao avaliar ε grad(1/ε).");
    }

    const Vector2 grad_n = get_regular_refractive_index_gradient(x, y);
    return Vector2{
        -2.0 * grad_n.x / refractive_index,
        -2.0 * grad_n.y / refractive_index};
}

double Waveguide::get_k_squared(double x, double y) const {
    const double n = get_refractive_index(x, y);
    double k0 = get_k0();
    return k0 * k0 * n * n;
}
