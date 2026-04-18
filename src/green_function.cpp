#include "green_function_internal.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>
#include <stdexcept>

namespace {

constexpr double kIntegrationUpperBound = 1.0 - 1e-9;
constexpr double kIntegrationTolerance = 1e-8;
constexpr int kMaxAdaptiveDepth = 18;

struct BundleIntegrandSample {
    double value = 0.0;
    double d_dx_source = 0.0;
    double d_dy_source = 0.0;
};

GreenFunctionPerformanceStats g_green_function_stats;

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

BundleIntegrandSample operator+(const BundleIntegrandSample& lhs, const BundleIntegrandSample& rhs) {
    return BundleIntegrandSample{
        lhs.value + rhs.value,
        lhs.d_dx_source + rhs.d_dx_source,
        lhs.d_dy_source + rhs.d_dy_source};
}

BundleIntegrandSample operator-(const BundleIntegrandSample& lhs, const BundleIntegrandSample& rhs) {
    return BundleIntegrandSample{
        lhs.value - rhs.value,
        lhs.d_dx_source - rhs.d_dx_source,
        lhs.d_dy_source - rhs.d_dy_source};
}

BundleIntegrandSample operator*(double scalar, const BundleIntegrandSample& sample) {
    return BundleIntegrandSample{
        scalar * sample.value,
        scalar * sample.d_dx_source,
        scalar * sample.d_dy_source};
}

double max_component_abs(const BundleIntegrandSample& sample) {
    return std::max({std::abs(sample.value), std::abs(sample.d_dx_source), std::abs(sample.d_dy_source)});
}

BundleIntegrandSample simpson_bundle(const std::function<BundleIntegrandSample(double)>& function,
                                     double a,
                                     double b) {
    const double c = 0.5 * (a + b);
    return ((b - a) / 6.0) * (function(a) + 4.0 * function(c) + function(b));
}

BundleIntegrandSample adaptive_simpson_recursive_bundle(const std::function<BundleIntegrandSample(double)>& function,
                                                        double a,
                                                        double b,
                                                        double tolerance,
                                                        const BundleIntegrandSample& whole,
                                                        int depth) {
    const double c = 0.5 * (a + b);
    const BundleIntegrandSample left = simpson_bundle(function, a, c);
    const BundleIntegrandSample right = simpson_bundle(function, c, b);
    const BundleIntegrandSample delta = (left + right) - whole;

    if (depth <= 0 || max_component_abs(delta) <= 15.0 * tolerance) {
        return left + right + (1.0 / 15.0) * delta;
    }

    return adaptive_simpson_recursive_bundle(function, a, c, tolerance * 0.5, left, depth - 1) +
           adaptive_simpson_recursive_bundle(function, c, b, tolerance * 0.5, right, depth - 1);
}

BundleIntegrandSample integrate_finite_interval_bundle(const std::function<BundleIntegrandSample(double)>& function,
                                                       double a,
                                                       double b,
                                                       double tolerance) {
    const BundleIntegrandSample whole = simpson_bundle(function, a, b);
    return adaptive_simpson_recursive_bundle(function, a, b, tolerance, whole, kMaxAdaptiveDepth);
}

BundleIntegrandSample integrate_positive_half_line_bundle(
    const std::function<BundleIntegrandSample(double)>& nu_integrand) {
    const auto transformed = [&](double t) {
        const double clamped_t = std::min(t, kIntegrationUpperBound);
        const double one_minus_t = 1.0 - clamped_t;
        const double nu = clamped_t / one_minus_t;
        const double jacobian = 1.0 / (one_minus_t * one_minus_t);
        return jacobian * nu_integrand(nu);
    };

    const BundleIntegrandSample whole = simpson_bundle(transformed, 0.0, kIntegrationUpperBound);
    return adaptive_simpson_recursive_bundle(
        transformed, 0.0, kIntegrationUpperBound, kIntegrationTolerance, whole, kMaxAdaptiveDepth);
}

BundleIntegrandSample integrate_oscillatory_positive_half_line_bundle(
    const std::function<BundleIntegrandSample(double)>& nu_integrand,
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

    BundleIntegrandSample total;
    for (double chunk_start = 0.0; chunk_start < nu_max; chunk_start += chunk_length) {
        const double chunk_end = std::min(chunk_start + chunk_length, nu_max);
        total = total + integrate_finite_interval_bundle(
                            nu_integrand, chunk_start, chunk_end, 0.25 * kIntegrationTolerance);
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

BundleIntegrandSample evaluate_ns_integrand(double nu,
                                            double delta_x,
                                            double sum_y,
                                            double beta,
                                            const Waveguide& wg) {
    const double gamma1_value = gamma_1(nu, beta, wg);
    const double gamma3_value = gamma_3(nu, beta, wg);
    const double reflection = reflection_coefficient(gamma1_value, gamma3_value);
    const double envelope = std::exp(-gamma3_value * sum_y);
    const double phase = 2.0 * PI * nu * delta_x;
    const double cosine = std::cos(phase);
    const double amplitude = (reflection / gamma3_value) * envelope;

    return BundleIntegrandSample{
        amplitude * cosine,
        amplitude * (2.0 * PI * nu * std::sin(phase)),
        -reflection * envelope * cosine};
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
    ++g_green_function_stats.g_ns_value_requests;
    return calculate_G_NS_bundle_internal(x, y, xp, yp, beta, wg).value;
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
    ++g_green_function_stats.g_ns_dx_requests;
    return calculate_G_NS_bundle_internal(x, y, xp, yp, beta, wg).d_dx_source;
}

Complex calculate_dG_NS_dy_source(double x, double y, double xp, double yp, double beta, const Waveguide& wg) {
    validate_supported_regime(y, yp, beta, wg);

    ++g_green_function_stats.g_ns_dy_requests;
    return calculate_G_NS_bundle_internal(x, y, xp, yp, beta, wg).d_dy_source;
}

GreenFunctionNonSingularBundle calculate_G_NS_bundle_internal(
    double x,
    double y,
    double xp,
    double yp,
    double beta,
    const Waveguide& wg) {
    validate_supported_regime(y, yp, beta, wg);

    ++g_green_function_stats.g_ns_bundle_evaluations;

    const double delta_x = x - xp;
    const double sum_y = y + yp;
    const auto integrand = [&](double nu) {
        return evaluate_ns_integrand(nu, delta_x, sum_y, beta, wg);
    };

    BundleIntegrandSample integral;
    if (std::abs(delta_x) > 1e-15) {
        ++g_green_function_stats.oscillatory_branch_evaluations;
        integral = integrate_oscillatory_positive_half_line_bundle(integrand, delta_x, sum_y);
    } else {
        ++g_green_function_stats.transformed_branch_evaluations;
        integral = integrate_positive_half_line_bundle(integrand);
    }

    return GreenFunctionNonSingularBundle{
        Complex(integral.value, 0.0),
        Complex(integral.d_dx_source, 0.0),
        Complex(integral.d_dy_source, 0.0)};
}

GreenFunctionPerformanceStats get_green_function_performance_stats() {
    return g_green_function_stats;
}

void reset_green_function_performance_stats() {
    g_green_function_stats = GreenFunctionPerformanceStats{};
}
