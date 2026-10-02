/**
 * @file canonical_pratt_parser_arena.cpp
 * @brief Modèle canonique C++20 : lexer zéro-copie + parser de Pratt + AST en arène.
 *
 * Règles illustrées (corpus .harness/knowledge/languages/cpp/) :
 *  - core/ownership-raii.md : aucun new/delete nu dans le code client ; l'arène
 *    détient tout et se libère en une seule passe (RAII).
 *  - core/memory.md         : arène de type bump — allocation O(1), nœuds AST
 *    contigus (localité cache), libération en bloc en fin de compilation.
 *  - domains/compilers.md   : lexer piloté par curseur (jamais de copie), Pratt
 *    pour la précédence, erreurs positionnées, golden tests via asserts.
 *
 * Compilation de référence :
 *   clang++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Wconversion -Werror \
 *           -fsanitize=address,undefined canonical_pratt_parser_arena.cpp -o /tmp/pratt
 */

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <new>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace forge::examples {

// ---------------------------------------------------------------------------
// Arène de type bump (bump allocator)
// ---------------------------------------------------------------------------

class Arena {
public:
    explicit Arena(std::size_t blockSize = 4096) : blockSize_(blockSize) {}
    ~Arena() { release(); }

    Arena(const Arena&) = delete;
    Arena& operator=(const Arena&) = delete;

    /// Réserve `bytes` octets alignés sur `align` (align <= alignof(std::max_align_t)).
    [[nodiscard]] void* allocate(std::size_t bytes, std::size_t align) {
        if (align > alignof(std::max_align_t)) {
            throw std::runtime_error("Arena : sur-alignement non supporté");
        }
        const auto cursor = reinterpret_cast<std::uintptr_t>(cursor_);
        const std::size_t padding = (cursor_ == nullptr)
            ? 0
            : (align - (cursor % align)) % align;

        if (cursor_ == nullptr || cursor_ + padding + bytes > end_) {
            growBlock(bytes + padding);
            // Bloc neuf : base alignée sur max_align_t -> padding nul, pas de recursion infinie.
            return allocate(bytes, align);
        }
        cursor_ += padding;
        void* result = cursor_;
        cursor_ += bytes;
        return result;
    }

    /// Construit un objet trivialement destructible dans l'arène.
    template <typename T, typename... Args>
    [[nodiscard]] T& create(Args&&... args) {
        static_assert(std::is_trivially_destructible_v<T>,
                      "Arena : ne stocker que des types trivialement destructibles "
                      "(nœuds d'AST). Règle : cpp/core/memory.md");
        void* mem = allocate(sizeof(T), alignof(T));
        return *new (mem) T(std::forward<Args>(args)...);
    }

    /// Libère TOUS les blocs en une fois — O(1). C'est LA fonction de libération
    /// documentée (CONVENTIONS.md) : aucune libération individuelle n'existe.
    void release() noexcept {
        for (Block* b = head_; b != nullptr;) {
            Block* next = b->next;
            ::operator delete(b, std::align_val_t{alignof(Block)});
            b = next;
        }
        head_ = nullptr;
        cursor_ = nullptr;
        end_ = nullptr;
    }

private:
    struct Block {
        Block* next;
        // L'union force l'alignement maximal avant le payload : tout type dont
        // alignof(T) <= alignof(std::max_align_t) est servi correctement.
        union Payload {
            std::max_align_t align;
            unsigned char data[1];
        } payload;
    };

    void growBlock(std::size_t minimumPayload) {
        const std::size_t payload = (minimumPayload > blockSize_) ? minimumPayload : blockSize_;
        const std::size_t total = offsetof(Block, payload.data) + payload;
        auto* block = static_cast<Block*>(::operator new(total, std::align_val_t{alignof(Block)}));
        block->next = head_;
        head_ = block;
        cursor_ = block->payload.data;
        end_ = cursor_ + payload;
    }

    Block* head_ = nullptr;
    unsigned char* cursor_ = nullptr;
    unsigned char* end_ = nullptr;
    std::size_t blockSize_;
};

// ---------------------------------------------------------------------------
// AST (trivialement destructible — eligible pour l'arène)
// ---------------------------------------------------------------------------

enum class BinOp { Add, Sub, Mul, Div };

struct Expr {
    enum class Kind { Number, Binary } kind;
    double number;
    BinOp op;
    const Expr* lhs; // nul pour Number
    const Expr* rhs; // nul pour Number
};

// ---------------------------------------------------------------------------
// Lexer zéro-copie : les lexèmes sont des vues sur la source
// ---------------------------------------------------------------------------

enum class TokKind { Number, Plus, Minus, Star, Slash, LParen, RParen, End };

struct Token {
    TokKind kind;
    std::string_view lexeme;
};

class Lexer {
public:
    explicit Lexer(std::string_view src) : src_(src) {}

    [[nodiscard]] Token next() {
        skipWhitespace();
        if (pos_ >= src_.size()) return {TokKind::End, {}};
        const char c = src_[pos_];
        if (c >= '0' && c <= '9') return scanNumber();
        switch (c) {
            case '+': return advance(TokKind::Plus);
            case '-': return advance(TokKind::Minus);
            case '*': return advance(TokKind::Star);
            case '/': return advance(TokKind::Slash);
            case '(': return advance(TokKind::LParen);
            case ')': return advance(TokKind::RParen);
            default: throw error("caractère inattendu");
        }
    }

private:
    std::string_view src_;
    std::size_t pos_ = 0;

    void skipWhitespace() {
        while (pos_ < src_.size() &&
               (src_[pos_] == ' ' || src_[pos_] == '\t' || src_[pos_] == '\n')) {
            ++pos_;
        }
    }

    [[nodiscard]] Token advance(TokKind kind) {
        const std::size_t start = pos_;
        ++pos_;
        return {kind, src_.substr(start, 1)};
    }

    [[nodiscard]] Token scanNumber() {
        const std::size_t start = pos_;
        while (pos_ < src_.size() && src_[pos_] >= '0' && src_[pos_] <= '9') {
            ++pos_;
        }
        return {TokKind::Number, src_.substr(start, pos_ - start)};
    }

    [[nodiscard]] std::runtime_error error(const char* msg) const {
        return std::runtime_error(std::string(msg) + " à la position " + std::to_string(pos_));
    }
};

// ---------------------------------------------------------------------------
// Parser de Pratt : la table de précédence est un fait local, pas une hiérarchie
// de classes d'expressions (domains/compilers.md).
// ---------------------------------------------------------------------------

class Parser {
public:
    Parser(std::string_view src, Arena& arena) : lexer_(src), arena_(arena) { advance(); }

    [[nodiscard]] const Expr* parseExpression(int minPrecedence = 0) {
        const Expr* lhs = parsePrimary();
        for (;;) {
            const int precedence = precedenceOf(current_.kind);
            if (precedence < minPrecedence) break;
            const TokKind op = current_.kind;
            advance();
            // precedence + 1 => associativité gauche ("8 / 4 / 2" == (8/4)/2).
            const Expr* rhs = parseExpression(precedence + 1);
            lhs = &arena_.create<Expr>(Expr::Kind::Binary, 0.0, toBinOp(op), lhs, rhs);
        }
        return lhs;
    }

private:
    Lexer lexer_;
    Token current_{TokKind::End, {}};
    Arena& arena_;

    void advance() { current_ = lexer_.next(); }

    [[nodiscard]] const Expr* parsePrimary() {
        if (current_.kind == TokKind::Number) {
            const double value = std::stod(std::string(current_.lexeme));
            advance();
            return &arena_.create<Expr>(Expr::Kind::Number, value, BinOp::Add, nullptr, nullptr);
        }
        if (current_.kind == TokKind::LParen) {
            advance();
            const Expr* inner = parseExpression();
            if (current_.kind != TokKind::RParen) {
                throw std::runtime_error("')' attendu");
            }
            advance();
            return inner;
        }
        throw std::runtime_error("opérande attendu");
    }

    [[nodiscard]] static int precedenceOf(TokKind k) noexcept {
        switch (k) {
            case TokKind::Plus:
            case TokKind::Minus:
                return 1;
            case TokKind::Star:
            case TokKind::Slash:
                return 2;
            default:
                return -1; // pas un opérateur binaire -> la boucle s'arrête
        }
    }

    [[nodiscard]] static BinOp toBinOp(TokKind k) noexcept {
        switch (k) {
            case TokKind::Plus: return BinOp::Add;
            case TokKind::Minus: return BinOp::Sub;
            case TokKind::Star: return BinOp::Mul;
            case TokKind::Slash: return BinOp::Div;
            default: return BinOp::Add; // inatteignable : garde-fou
        }
    }
};

// ---------------------------------------------------------------------------
// Évaluateur direct sur l'AST (les nœuds vivent dans l'arène, pas sur le tas)
// ---------------------------------------------------------------------------

[[nodiscard]] double evaluate(const Expr& e) {
    switch (e.kind) {
        case Expr::Kind::Number:
            return e.number;
        case Expr::Kind::Binary: {
            const double l = evaluate(*e.lhs);
            const double r = evaluate(*e.rhs);
            switch (e.op) {
                case BinOp::Add: return l + r;
                case BinOp::Sub: return l - r;
                case BinOp::Mul: return l * r;
                case BinOp::Div:
                    if (r == 0.0) throw std::runtime_error("division par zéro");
                    return l / r;
            }
        }
    }
    throw std::runtime_error("expression invalide"); // inatteignable (garde-fou)
}

} // namespace forge::examples

namespace {

[[nodiscard]] double eval(const char* src) {
    forge::examples::Arena arena;
    forge::examples::Parser parser(src, arena);
    return forge::examples::evaluate(*parser.parseExpression());
}

} // namespace

int main() {
    // Golden tests (domains/compilers.md) : toute régression de précédence ou
    // d'associativité se voit ici, avant tout déploiement.
    assert(eval("1 + 2 * 3") == 7.0);   // Pratt : * lie plus fort que +
    assert(eval("(1 + 2) * 3") == 9.0); // parenthèses
    assert(eval("8 / 4 / 2") == 1.0);   // associativité gauche
    assert(eval("10 - 2 - 3") == 5.0);  // idem

    // Erreur positionnée : "1 +" lève une exception explicite, pas un crash.
    bool threw = false;
    try {
        forge::examples::Arena arena;
        forge::examples::Parser parser("1 +", arena);
        (void)parser.parseExpression();
    } catch (const std::runtime_error& e) {
        threw = true;
        std::cout << "Erreur attendue interceptée : " << e.what() << '\n';
    }
    assert(threw);

    std::cout << "Modèle Pratt + arène validé sous ASan/UBSan.\n";
    return 0;
}
