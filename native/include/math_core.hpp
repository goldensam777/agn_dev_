#ifndef FORGE_MATH_CORE_HPP
#define FORGE_MATH_CORE_HPP

#include <vector>
#include <cstddef>

namespace forge::native {

/**
 * Calcul du produit scalaire de deux vecteurs à haute vitesse (SIMD-friendly).
 * Aucune allocation dynamique cachée.
 */
double dot_product(const std::vector<double>& a, const std::vector<double>& b);

/**
 * Estimation de Pi par méthode Monte Carlo haute performance.
 * Utilise un générateur pseudo-aléatoire à faible état.
 */
double monte_carlo_pi(std::size_t iterations, unsigned int seed = 42);

/**
 * Multiplication matricielle naïve pour étalonner le banc Mercuria (O(N^3)).
 */
std::vector<double> square_matrix_multiply(
    const std::vector<double>& a,
    const std::vector<double>& b,
    std::size_t n
);

} // namespace forge::native

#endif // FORGE_MATH_CORE_HPP
