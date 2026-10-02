/**
 * @file canonical_kahan_summation.cpp
 * @brief Modèle canonique C++20 : sommation compensée (Kahan / Neumaier).
 *
 * Règles illustrées (corpus .harness/knowledge/languages/cpp/) :
 *  - domains/scientific.md : la cancellation est un bug de précision — quantifier
 *    l'erreur contre une référence plus précise, jamais l'espérer.
 *  - core/memory.md        : plage contiguë (std::span), accès strictement séquentiel.
 *  - CONVENTIONS.md        : performance mesurée, jamais présumée → on AFFICHE
 *    l'erreur réelle de chaque variante au lieu de deviner laquelle est bonne.
 *
 * Compilation : clang++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror \
 *               -fsanitize=address,undefined canonical_kahan_summation.cpp -o /tmp/kahan
 */

#include <cmath>
#include <cstddef>
#include <iostream>
#include <span>
#include <vector>

namespace forge::examples {

// Variante naïve : référence de simplicité, PAS de qualité. Sert de contre-exemple mesuré.
[[nodiscard]] double sum_naive(std::span<const double> xs) noexcept {
    double total = 0.0;
    for (const double x : xs) {
        total += x;
    }
    return total;
}

// Kahan : compense l'erreur d'arrondi à chaque addition. Séquentiel PAR CONSTRUCTION
// (la compensation empêche la vectorisation — compromis EXPLICITE, à benchmarker).
[[nodiscard]] double sum_kahan(std::span<const double> xs) noexcept {
    double sum = 0.0;
    double compensation = 0.0;
    for (const double x : xs) {
        const double y = x - compensation;
        const double t = sum + y;
        compensation = (t - sum) - y; // ce qui vient d'être perdu dans l'arrondi de t
        sum = t;
    }
    return sum;
}

// Neumaier (amélioration de Kahan) : gère mieux le cas |x| >> |sum|.
[[nodiscard]] double sum_neumaier(std::span<const double> xs) noexcept {
    double sum = 0.0;
    double compensation = 0.0;
    for (const double x : xs) {
        const double t = sum + x;
        if (std::fabs(sum) >= std::fabs(x)) {
            compensation += (sum - t) + x;
        } else {
            compensation += (x - t) + sum;
        }
        sum = t;
    }
    return sum + compensation;
}

} // namespace forge::examples

int main() {
    // Cas adversarial : une grande valeur suivie de milliers de petites.
    // La somme naïve perd systématiquement les petits termes (cancellation).
    std::vector<double> xs;
    xs.reserve(10'000);
    xs.push_back(1.0e16);
    for (int i = 0; i < 9'999; ++i) {
        xs.push_back(1.0 + 1.0e-9 * static_cast<double>(i % 7));
    }

    // Référence en précision étendue : l'arbitre de l'expérience.
    const long double reference = [&] {
        long double r = 0.0L;
        for (const double x : xs) {
            r += static_cast<long double>(x);
        }
        return r;
    }();

    const auto report = [&](const char* name, double value) {
        const double err = std::fabs(static_cast<double>(reference) - value);
        std::cout << name << " : " << value
                  << "  |erreur vs réf. long double| = " << err << '\n';
    };

    report("naive   ", forge::examples::sum_naive(xs));
    report("kahan   ", forge::examples::sum_kahan(xs));
    report("neumaier", forge::examples::sum_neumaier(xs));

    std::cout << "Règle : choisir la variante sur l'erreur MESURÉE, pas sur l'intuition.\n";
    return 0;
}
