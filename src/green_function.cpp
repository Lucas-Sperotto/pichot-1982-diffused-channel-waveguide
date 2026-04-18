#include "green_function.h"

#include <cmath>
#include <functional>
#include <limits>
#include <stdexcept>

namespace {

constexpr double kIntegrationUpperBound = 1.0 - 1e-9;
constexpr double kIntegrationTolerance = 1e-8;
constexpr int kMaxAdaptiveDepth = 18;

double squared(double value) {
    return value * value;
}

double get_k1_squared(const Waveguide& wg) {
    const double k0 = wg.get_k0();
    return squared(k0 * wg.get_params().n1);
}

double get_k3_squared(const Waveguide& wg) {
    const double k0 = wg.get_k0();
    return squared(k0 * wg.get_params().n3);
}

double guided_alpha(double beta, double k_squared, const char* label) {
    const double value = beta * beta - k_squared;
    if (value <= 0.0) {
        throw std::runtime_error(std::string("Regime não guiado ou degenerado para ") + label + ".");
    }
    return std::sqrt(value);
}

void validate_supported_regime(double y, double yp, double beta, const Waveguide& wg) {
    if (y < 0.0 || yp < 0.0) {
        throw std::runtime_error(
            "A implementação atual da função de Green cobre apenas pontos com y >= 0 e y' >= 0.");
    }

    (void)guided_alpha(beta, get_k1_squared(wg), "gamma_1");
    (void)guided_alpha(beta, get_k3_squared(wg), "gamma_3");
}

double gamma_1(double nu, double beta, const Waveguide& wg) {
    return std::sqrt(4.0 * PI * PI * nu * nu + beta * beta - get_k1_squared(wg));
}

double gamma_3(double nu, double beta, const Waveguide& wg) {
    return std::sqrt(4.0 * PI * PI * nu * nu + beta * beta - get_k3_squared(wg));
}

double simpson(const std::function<double(double)>& function, double a, double b) {
    const double c = 0.5 * (a + b);
    return (b - a) * (function(a) + 4.0 * function(c) + function(b)) / 6.0;
}

double adaptive_simpson_recursive(const std::function<double(double)>& function,
                                  double a,
                                  double b,
                                  double tolerance,
                                  double whole,
                                  int depth) {
    const double c = 0.5 * (a + b);
    const double left = simpson(function, a, c);
    const double right = simpson(function, c, b);
    const double delta = left + right - whole;

    if (depth <= 0 || std::abs(delta) <= 15.0 * tolerance) {
        return left + right + delta / 15.0;
    }

    return adaptive_simpson_recursive(function, a, c, tolerance * 0.5, left, depth - 1) +
           adaptive_simpson_recursive(function, c, b, tolerance * 0.5, right, depth - 1);
}

double integrate_finite_interval(const std::function<double(double)>& function,
                                 double a,
                                 double b,
                                 double tolerance) {
    const double whole = simpson(function, a, b);
    return adaptive_simpson_recursive(function, a, b, tolerance, whole, kMaxAdaptiveDepth);
}

double integrate_positive_half_line(const std::function<double(double)>& nu_integrand) {
    const auto transformed = [&](double t) {
        const double clamped_t = std::min(t, kIntegrationUpperBound);
        const double one_minus_t = 1.0 - clamped_t;
        const double nu = clamped_t / one_minus_t;
        const double jacobian = 1.0 / (one_minus_t * one_minus_t);
        return nu_integrand(nu) * jacobian;
    };

    const double whole = simpson(transformed, 0.0, kIntegrationUpperBound);
    return adaptive_simpson_recursive(
        transformed, 0.0, kIntegrationUpperBound, kIntegrationTolerance, whole, kMaxAdaptiveDepth);
}

double integrate_oscillatory_positive_half_line(const std::function<double(double)>& nu_integrand,
                                                double delta_x,
                                                double sum_y) {
    const double decay_cutoff =
        sum_y > 0.0 ? -std::log(0.1 * kIntegrationTolerance) / (2.0 * PI * sum_y) : 0.0;
    const double phase_cutoff = std::abs(delta_x) > 1e-15 ? 4.0 / std::abs(delta_x) : 0.0;
    const double nu_max = std::max({decay_cutoff, phase_cutoff, 1.0});
    const double oscillation_chunk =
        std::abs(delta_x) > 1e-15 ? 0.25 / std::abs(delta_x) : std::numeric_limits<double>::infinity();
    const double decay_chunk =
        sum_y > 0.0 ? 0.5 / (2.0 * PI * sum_y) : std::numeric_limits<double>::infinity();
    const double base_chunk = std::min(oscillation_chunk, decay_chunk);
    const double chunk_length = std::min(nu_max, std::max(base_chunk, 1e3));

    double total = 0.0;
    for (double chunk_start = 0.0; chunk_start < nu_max; chunk_start += chunk_length) {
        const double chunk_end = std::min(chunk_start + chunk_length, nu_max);
        total += integrate_finite_interval(nu_integrand, chunk_start, chunk_end, 0.25 * kIntegrationTolerance);
    }
    return total;
}

double radial_distance(double x, double y, double xp, double yp) {
    return std::hypot(x - xp, y - yp);
}

Complex singular_value() {
    return Complex(std::numeric_limits<double>::infinity(), 0.0);
}

double reflection_coefficient(double gamma1, double gamma3) {
    return (gamma3 - gamma1) / (gamma3 + gamma1);
}

} // namespace

Complex calculate_G_S(double x, double y, double xp, double yp, double beta, const Waveguide& wg) {
    validate_supported_regime(y, yp, beta, wg);

    const double rho = radial_distance(x, y, xp, yp);
    if (rho <= 0.0) {
        return singular_value();
    }

    const double alpha_3 = guided_alpha(beta, get_k3_squared(wg), "gamma_3");
    const double argument = alpha_3 * rho;
    const double value = std::cyl_bessel_k(0.0, argument) / (2.0 * PI);

    return Complex(value, 0.0);
}

Complex calculate_G_NS(double x, double y, double xp, double yp, double beta, const Waveguide& wg) {
    validate_supported_regime(y, yp, beta, wg);

    const double delta_x = x - xp;
    const double sum_y = y + yp;
    const auto integrand = [&](double nu) {
        const double gamma1 = gamma_1(nu, beta, wg);
        const double gamma3 = gamma_3(nu, beta, wg);
        const double reflection = reflection_coefficient(gamma1, gamma3);
        const double envelope = std::exp(-gamma3 * sum_y);
        return (reflection / gamma3) * envelope * std::cos(2.0 * PI * nu * delta_x);
    };

    if (std::abs(delta_x) > 1e-15) {
        return Complex(integrate_oscillatory_positive_half_line(integrand, delta_x, sum_y), 0.0);
    }

    return Complex(integrate_positive_half_line(integrand), 0.0);
}

Complex calculate_G(double x, double y, double xp, double yp, double beta, const Waveguide& wg) {
    return calculate_G_S(x, y, xp, yp, beta, wg) + calculate_G_NS(x, y, xp, yp, beta, wg);
}

Complex calculate_dG_S_dx_source(double x, double y, double xp, double yp, double beta, const Waveguide& wg) {
    validate_supported_regime(y, yp, beta, wg);

    const double delta_x = x - xp;
    const double rho = radial_distance(x, y, xp, yp);
    if (rho <= 0.0) {
        return singular_value();
    }

    const double alpha_3 = guided_alpha(beta, get_k3_squared(wg), "gamma_3");
    const double argument = alpha_3 * rho;
    const double value = alpha_3 * std::cyl_bessel_k(1.0, argument) * delta_x / (2.0 * PI * rho);

    return Complex(value, 0.0);
}

Complex calculate_dG_S_dy_source(double x, double y, double xp, double yp, double beta, const Waveguide& wg) {
    validate_supported_regime(y, yp, beta, wg);

    const double delta_y = y - yp;
    const double rho = radial_distance(x, y, xp, yp);
    if (rho <= 0.0) {
        return singular_value();
    }

    const double alpha_3 = guided_alpha(beta, get_k3_squared(wg), "gamma_3");
    const double argument = alpha_3 * rho;
    const double value = alpha_3 * std::cyl_bessel_k(1.0, argument) * delta_y / (2.0 * PI * rho);

    return Complex(value, 0.0);
}

Complex calculate_dG_NS_dx_source(double x, double y, double xp, double yp, double beta, const Waveguide& wg) {
    validate_supported_regime(y, yp, beta, wg);

    const double delta_x = x - xp;
    const double sum_y = y + yp;
    const auto integrand = [&](double nu) {
        const double gamma1 = gamma_1(nu, beta, wg);
        const double gamma3 = gamma_3(nu, beta, wg);
        const double reflection = reflection_coefficient(gamma1, gamma3);
        const double envelope = std::exp(-gamma3 * sum_y);
        const double phase_derivative = 2.0 * PI * nu * std::sin(2.0 * PI * nu * delta_x);
        return (reflection / gamma3) * envelope * phase_derivative;
    };

    if (std::abs(delta_x) > 1e-15) {
        return Complex(integrate_oscillatory_positive_half_line(integrand, delta_x, sum_y), 0.0);
    }

    return Complex(integrate_positive_half_line(integrand), 0.0);
}

Complex calculate_dG_NS_dy_source(double x, double y, double xp, double yp, double beta, const Waveguide& wg) {
    validate_supported_regime(y, yp, beta, wg);

    const double delta_x = x - xp;
    const double sum_y = y + yp;
    const auto integrand = [&](double nu) {
        const double gamma1 = gamma_1(nu, beta, wg);
        const double gamma3 = gamma_3(nu, beta, wg);
        const double reflection = reflection_coefficient(gamma1, gamma3);
        const double envelope = std::exp(-gamma3 * sum_y);
        return -reflection * envelope * std::cos(2.0 * PI * nu * delta_x);
    };

    if (std::abs(delta_x) > 1e-15) {
        return Complex(integrate_oscillatory_positive_half_line(integrand, delta_x, sum_y), 0.0);
    }

    return Complex(integrate_positive_half_line(integrand), 0.0);
}
