#include "math_core.hpp"
#include <stdexcept>
#include <random>

namespace forge::native {

double dot_product(const std::vector<double>& a, const std::vector<double>& b) {
    if (a.size() != b.size()) {
        throw std::invalid_argument("Les dimensions des vecteurs doivent correspondre.");
    }
    double sum = 0.0;
    const std::size_t n = a.size();
    for (std::size_t i = 0; i < n; ++i) {
        sum += a[i] * b[i];
    }
    return sum;
}

double monte_carlo_pi(std::size_t iterations, unsigned int seed) {
    if (iterations == 0) {
        return 0.0;
    }
    std::mt19937_64 rng(seed);
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    std::size_t inside_circle = 0;
    for (std::size_t i = 0; i < iterations; ++i) {
        const double x = dist(rng);
        const double y = dist(rng);
        if (x * x + y * y <= 1.0) {
            ++inside_circle;
        }
    }
    return 4.0 * static_cast<double>(inside_circle) / static_cast<double>(iterations);
}

std::vector<double> square_matrix_multiply(
    const std::vector<double>& a,
    const std::vector<double>& b,
    std::size_t n
) {
    if (a.size() != n * n || b.size() != n * n) {
        throw std::invalid_argument("La taille des vecteurs ne correspond pas à la dimension n*n.");
    }

    std::vector<double> c(n * n, 0.0);
    for (std::size_t i = 0; i < n; ++i) {
        for (std::size_t k = 0; k < n; ++k) {
            const double a_ik = a[i * n + k];
            for (std::size_t j = 0; j < n; ++j) {
                c[i * n + j] += a_ik * b[k * n + j];
            }
        }
    }
    return c;
}

} // namespace forge::native
