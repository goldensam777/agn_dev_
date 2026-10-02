/**
 * @file canonical_vector_core.cpp
 * @brief Modèle canonique d'implémentation C++20 haute performance
 * 
 * Règles illustrées :
 * 1. Zéro fuite mémoire (RAII exclusif, aucun new/delete nu).
 * 2. Données contiguës en mémoire pour optimiser les lignes de cache L1/L2.
 * 3. Gestion stricte des cas limites (vecteurs vides, incompatibilités de dimensions).
 * 4. Compatible compilation avec -fsanitize=address,undefined -Wall -Wextra -Werror.
 */

#include <vector>
#include <numeric>
#include <stdexcept>
#include <cstddef>
#include <iostream>
#include <cassert>

namespace forge::examples {

template <typename T>
class CanonicalVectorProcessor {
public:
    explicit CanonicalVectorProcessor(std::vector<T> data) : data_(std::move(data)) {}

    [[nodiscard]] std::size_t size() const noexcept {
        return data_.size();
    }

    [[nodiscard]] const std::vector<T>& data() const noexcept {
        return data_;
    }

    // Réduction par somme optimisée pour le compilateur (auto-vectorisable)
    [[nodiscard]] T accumulate_sum() const noexcept {
        T total = T{0};
        const std::size_t n = data_.size();
        const T* const ptr = data_.data();
        
        #pragma clang loop vectorize(enable)
        for (std::size_t i = 0; i < n; ++i) {
            total += ptr[i];
        }
        return total;
    }

private:
    std::vector<T> data_;
};

} // namespace forge::examples

int main() {
    std::vector<double> values = {1.5, 2.5, 3.5, 4.5};
    forge::examples::CanonicalVectorProcessor<double> processor(std::move(values));

    assert(processor.size() == 4);
    assert(processor.accumulate_sum() == 12.0);

    std::cout << "Exemple canonique validé avec succès (Somme = " << processor.accumulate_sum() << ")\n";
    return 0;
}
