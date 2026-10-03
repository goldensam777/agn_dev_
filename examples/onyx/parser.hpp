#pragma once

#include "arena.hpp"
#include "ast.hpp"
#include "token.hpp"
#include <vector>
#include <span>

namespace onyx {

class Parser {
public:
    Parser(std::span<const Token> tokens, Arena& arena);

    /**
     * @brief Parse le programme complet en un bloc racine d'instructions.
     */
    BlockStmt* parse_program();

    /**
     * @brief Parse une expression unique (utilisé pour les tests et REPL).
     */
    Expr* parse_expression(int min_bp = 0);

private:
    const Token& peek() const noexcept;
    const Token& previous() const noexcept;
    const Token& advance() noexcept;
    bool check(TokenKind kind) const noexcept;
    bool match(TokenKind kind) noexcept;
    const Token& consume(TokenKind kind, const std::string& error_msg);
    void skip_newlines() noexcept;

    // Analyse d'instructions
    Stmt* parse_statement();
    Stmt* parse_variable_declaration_or_assignment(bool is_persistent);
    Stmt* parse_if_statement();
    Stmt* parse_while_statement();
    Stmt* parse_match_statement();
    Stmt* parse_try_fallback_statement();
    Stmt* parse_function_declaration(std::string name);
    BlockStmt* parse_indented_block();

    // Analyse d'expressions (Pratt Parsing)
    Expr* parse_prefix();
    Expr* parse_infix(Expr* left, int min_bp);
    int get_infix_bp(TokenKind kind) const noexcept;

    // Analyse de types
    OnyxType parse_type();

    std::span<const Token> tokens_;
    size_t cursor_{0};
    Arena& arena_;
};

} // namespace onyx
