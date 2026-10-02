#include "math_core.hpp"
#include <iostream>
#include <cassert>
#include <cmath>

int main() {
    std::cout << "[TEST] Démarrage des tests natifs sous AddressSanitizer...\n";

    // 1. Test dot_product
    {
        std::vector<double> v1 = {1.0, 2.0, 3.0};
        std::vector<double> v2 = {4.0, -5.0, 6.0};
        double res = forge::native::dot_product(v1, v2);
        // 1*4 + 2*(-5) + 3*6 = 4 - 10 + 18 = 12
        assert(std::abs(res - 12.0) < 1e-9);
        std::cout << "  ✓ dot_product validé (résultat = " << res << ")\n";
    }

    // 2. Test monte_carlo_pi
    {
        std::size_t iters = 1'000'000;
        double pi_est = forge::native::monte_carlo_pi(iters, 12345);
        assert(pi_est > 3.10 && pi_est < 3.18);
        std::cout << "  ✓ monte_carlo_pi validé (" << iters << " iters, pi ≈ " << pi_est << ")\n";
    }

    // 3. Test matrix_multiply
    {
        std::size_t n = 2;
        std::vector<double> a = {1.0, 2.0, 3.0, 4.0};
        std::vector<double> b = {2.0, 0.0, 1.0, 2.0};
        auto c = forge::native::square_matrix_multiply(a, b, n);
        // [1 2; 3 4] * [2 0; 1 2] = [4 4; 10 8]
        assert(std::abs(c[0] - 4.0) < 1e-9);
        assert(std::abs(c[1] - 4.0) < 1e-9);
        assert(std::abs(c[2] - 10.0) < 1e-9);
        assert(std::abs(c[3] - 8.0) < 1e-9);
        std::cout << "  ✓ square_matrix_multiply validé (2x2)\n";
    }

    std::cout << "[TEST] Tous les tests natifs ont réussi sans erreur mémoire !\n";
    return 0;
}
