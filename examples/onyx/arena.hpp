#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <new>
#include <type_traits>
#include <utility>
#include <vector>

namespace onyx {

/**
 * @brief Allocateur d'arène contigu (Bump Allocator) pour le compilateur Onyx.
 * 
 * Permet des allocations ultra-rapides en O(1) pour les nœuds de l'AST et les
 * chaînes de tokens sans aucune fragmentation du tas ni coût de `free` individuel.
 * Libération globale en bloc avec invocation déterministe des destructeurs non-triviaux
 * (zéro fuite mémoire sous ASan).
 */
class Arena {
public:
    static constexpr size_t DEFAULT_CHUNK_SIZE = 64 * 1024; // 64 Ko par bloc

    explicit Arena(size_t chunk_size = DEFAULT_CHUNK_SIZE)
        : chunk_size_(chunk_size), current_ptr_(nullptr), remaining_(0) {}

    ~Arena() {
        reset();
    }

    Arena(const Arena&) = delete;
    Arena& operator=(const Arena&) = delete;

    Arena(Arena&& other) noexcept
        : chunk_size_(other.chunk_size_),
          chunks_(std::move(other.chunks_)),
          cleanups_(std::move(other.cleanups_)),
          current_ptr_(other.current_ptr_),
          remaining_(other.remaining_) {
        other.current_ptr_ = nullptr;
        other.remaining_ = 0;
    }

    Arena& operator=(Arena&& other) noexcept {
        if (this != &other) {
            reset();
            chunk_size_ = other.chunk_size_;
            chunks_ = std::move(other.chunks_);
            cleanups_ = std::move(other.cleanups_);
            current_ptr_ = other.current_ptr_;
            remaining_ = other.remaining_;
            other.current_ptr_ = nullptr;
            other.remaining_ = 0;
        }
        return *this;
    }

    /**
     * @brief Alloue un bloc d'octets avec alignement garanti.
     */
    [[nodiscard]] void* allocate(size_t size, size_t alignment = alignof(std::max_align_t)) {
        if (size == 0) return nullptr;

        size_t current_addr = reinterpret_cast<size_t>(current_ptr_);
        size_t offset = (alignment - (current_addr % alignment)) % alignment;
        size_t needed = size + offset;

        if (needed > remaining_) {
            allocate_chunk(std::max(chunk_size_, needed));
            current_addr = reinterpret_cast<size_t>(current_ptr_);
            offset = (alignment - (current_addr % alignment)) % alignment;
            needed = size + offset;
        }

        void* aligned_ptr = current_ptr_ + offset;
        current_ptr_ += needed;
        remaining_ -= needed;
        return aligned_ptr;
    }

    /**
     * @brief Instancie un objet de type T dans l'arène.
     * Enregistre le destructeur si T n'est pas trivialement destructible.
     */
    template <typename T, typename... Args>
    [[nodiscard]] T* create(Args&&... args) {
        void* mem = allocate(sizeof(T), alignof(T));
        T* obj = ::new (mem) T(std::forward<Args>(args)...);
        if constexpr (!std::is_trivially_destructible_v<T>) {
            cleanups_.push_back({[](void* p) { static_cast<T*>(p)->~T(); }, obj});
        }
        return obj;
    }

    /**
     * @brief Invoque tous les destructeurs enregistrés puis libère tous les blocs alloués.
     */
    void reset() noexcept {
        for (auto it = cleanups_.rbegin(); it != cleanups_.rend(); ++it) {
            it->fn(it->obj);
        }
        cleanups_.clear();

        for (void* chunk : chunks_) {
            std::free(chunk);
        }
        chunks_.clear();
        current_ptr_ = nullptr;
        remaining_ = 0;
    }

    [[nodiscard]] size_t total_chunks() const noexcept {
        return chunks_.size();
    }

private:
    struct Cleanup {
        void (*fn)(void*);
        void* obj;
    };

    void allocate_chunk(size_t size) {
        void* chunk = std::malloc(size);
        if (!chunk) {
            throw std::bad_alloc();
        }
        chunks_.push_back(chunk);
        current_ptr_ = static_cast<std::byte*>(chunk);
        remaining_ = size;
    }

    size_t chunk_size_;
    std::vector<void*> chunks_;
    std::vector<Cleanup> cleanups_;
    std::byte* current_ptr_;
    size_t remaining_;
};

} // namespace onyx
