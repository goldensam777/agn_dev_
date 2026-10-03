#pragma once

#include "token.hpp"
#include <string_view>
#include <vector>
#include <stack>

namespace onyx {

class Lexer {
public:
    explicit Lexer(std::string_view source);

    /**
     * @brief Retourne le prochain token du flux lexical.
     */
    Token next_token();

    /**
     * @brief Tokenise l'intégralité du code source en un vecteur.
     */
    std::vector<Token> tokenize_all();

private:
    char peek() const noexcept;
    char peek_next() const noexcept;
    char advance() noexcept;
    bool match(char expected) noexcept;
    bool is_at_end() const noexcept;

    void skip_whitespace_and_comments();
    void skip_comment();
    Token handle_newline_and_indentation();

    Token make_token(TokenKind kind, std::string text = "");
    Token number();
    Token identifier_or_keyword();
    Token string(char quote_char);

    std::string_view source_;
    size_t cursor_{0};
    size_t line_{1};
    size_t column_{1};

    std::vector<size_t> indent_stack_{0};
    size_t pending_dedents_{0};
    bool at_line_start_{true};
};

} // namespace onyx
