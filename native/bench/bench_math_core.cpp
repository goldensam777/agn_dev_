#include "math_core.hpp"
#include <iostream>
#include <chrono>
#include <vector>

int main() {
    std::cout << "=== MERCURIA BENCHMARK HARNESS ===\n";

    // Benchmark Dot Product sur 10 millions d'éléments
    const std::size_t N = 10'000'000;
    std::cout << "[MERCURIA] Banc dot_product avec N = " << N << " éléments...\n";

    std::vector<double> v1(N, 1.0001);
    std::vector<double> v2(N, 0.9999);

    auto start = std::chrono::high_resolution_clock::now();
    double res = forge::native::dot_product(v1, v2);
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> duration = end - start;
    double seconds = duration.count() / 1000.0;
    double mops = (static_cast<double>(N) / seconds) / 1'000'000.0;

    std::cout << "  Résultat : " << res << "\n";
    std::cout << "  Temps d'exécution : " << duration.count() << " ms\n";
    std::cout << "  Débit : " << mops << " Millions d'opérations/sec\n";
    std::cout << "=== FIN DU BANC MERCURIA ===\n";
    return 0;
}
