#include "parser.hpp"
#include <stdexcept>

namespace onyx {

Parser::Parser(std::span<const Token> tokens, Arena& arena)
    : tokens_(tokens), cursor_(0), arena_(arena) {}

const Token& Parser::peek() const noexcept {
    if (cursor_ >= tokens_.size()) return tokens_.back(); // Dernier token est EOF
    return tokens_[cursor_];
}

const Token& Parser::previous() const noexcept {
    if (cursor_ == 0) return tokens_[0];
    return tokens_[cursor_ - 1];
}

const Token& Parser::advance() noexcept {
    if (cursor_ < tokens_.size()) {
        cursor_++;
    }
    return previous();
}

bool Parser::check(TokenKind kind) const noexcept {
    return peek().kind == kind;
}

bool Parser::match(TokenKind kind) noexcept {
    if (check(kind)) {
        advance();
        return true;
    }
    return false;
}

const Token& Parser::consume(TokenKind kind, const std::string& error_msg) {
    if (check(kind)) return advance();
    throw std::runtime_error(error_msg + " (Reçu: " + token_kind_to_string(peek().kind) +
                             " à la ligne " + std::to_string(peek().span.line) + ")");
}

void Parser::skip_newlines() noexcept {
    while (check(TokenKind::Newline)) {
        advance();
    }
}

BlockStmt* Parser::parse_program() {
    auto* root = arena_.create<BlockStmt>();
    skip_newlines();
    while (!check(TokenKind::Eof)) {
        Stmt* s = parse_statement();
        if (s) {
            root->statements.push_back(s);
        }
        skip_newlines();
    }
    return root;
}

BlockStmt* Parser::parse_indented_block() {
    consume(TokenKind::Colon, "':' attendu avant un bloc indenté");
    skip_newlines();
    consume(TokenKind::Indent, "Indentation attendue après ':'");

    auto* block = arena_.create<BlockStmt>();
    skip_newlines();

    while (!check(TokenKind::Dedent) && !check(TokenKind::Eof)) {
        Stmt* s = parse_statement();
        if (s) {
            block->statements.push_back(s);
        }
        skip_newlines();
    }

    consume(TokenKind::Dedent, "Dédentation attendue à la fin du bloc");
    return block;
}

Stmt* Parser::parse_statement() {
    skip_newlines();
    if (check(TokenKind::Eof)) return nullptr;

    // Persistance !i
    if (check(TokenKind::BangI)) {
        advance(); // Consomme !i
        return parse_variable_declaration_or_assignment(true);
    }

    // Retour immédiat => expr
    if (match(TokenKind::ArrowFat)) {
        Expr* val = parse_expression();
        return arena_.create<ReturnStmt>(val);
    }

    // Sortie de boucle :break expr?
    if (match(TokenKind::BreakKw)) {
        Expr* val = nullptr;
        if (!check(TokenKind::Newline) && !check(TokenKind::Dedent) && !check(TokenKind::Eof)) {
            val = parse_expression();
        }
        return arena_.create<BreakStmt>(val);
    }

    // Conditionnelle if / elif / else
    if (match(TokenKind::IfKw)) {
        return parse_if_statement();
    }

    // Boucle while
    if (match(TokenKind::WhileKw)) {
        return parse_while_statement();
    }

    // Pattern matching match expr:
    if (match(TokenKind::MatchKw)) {
        return parse_match_statement();
    }

    // Gestion d'erreur try: ... fallback:
    if (match(TokenKind::TryKw)) {
        return parse_try_fallback_statement();
    }

    // Déclaration de fonction nommée ou variable ou affectation
    if (check(TokenKind::Identifier)) {
        std::string name = peek().text;

        // Regarder si c'est une fonction : name(...) -> Type:
        if (cursor_ + 1 < tokens_.size() && tokens_[cursor_ + 1].kind == TokenKind::LParen) {
            // Pourrait être un appel ou une déclaration de fonction
            // Si on a name(...) -> RetType: c'est une fonction
            size_t lookahead = cursor_ + 1;
            int parens = 0;
            bool is_func_decl = false;

            while (lookahead < tokens_.size()) {
                if (tokens_[lookahead].kind == TokenKind::LParen) parens++;
                else if (tokens_[lookahead].kind == TokenKind::RParen) {
                    parens--;
                    if (parens == 0) {
                        if (lookahead + 1 < tokens_.size() && tokens_[lookahead + 1].kind == TokenKind::ArrowThin) {
                            is_func_decl = true;
                        }
                        break;
                    }
                }
                lookahead++;
            }

            if (is_func_decl) {
                advance(); // Consomme le nom de la fonction
                return parse_function_declaration(name);
            }
        }

        // Sinon déclaration ou assignation de variable
        return parse_variable_declaration_or_assignment(false);
    }

    // Expression simple (ex: print(...) ou expr)
    Expr* e = parse_expression();
    return arena_.create<ExprStmt>(e);
}

Stmt* Parser::parse_variable_declaration_or_assignment(bool is_persistent) {
    std::string name = consume(TokenKind::Identifier, "Nom de variable attendu").text;

    // Indexation de tenseur pour affectation : tensor[i, j] = val
    if (check(TokenKind::LBracket)) {
        advance(); // [
        std::vector<Expr*> indices;
        indices.push_back(parse_expression());
        while (match(TokenKind::Comma)) {
            indices.push_back(parse_expression());
        }
        consume(TokenKind::RBracket, "']' attendu après les indices");
        consume(TokenKind::Assign, "'=' attendu pour l'assignation de cellule de tenseur");
        Expr* val = parse_expression();
        return arena_.create<TensorAssignStmt>(name, indices, val);
    }

    std::optional<OnyxType> type_annot;
    if (match(TokenKind::Colon)) {
        type_annot = parse_type();
    }

    if (match(TokenKind::Assign)) {
        Expr* init = parse_expression();
        return arena_.create<VarDeclStmt>(name, is_persistent, type_annot, init);
    }

    if (type_annot.has_value()) {
        // Déclaration sans valeur initiale explicite
        return arena_.create<VarDeclStmt>(name, is_persistent, type_annot, nullptr);
    }

    // Sinon c'était une expression identifiant isolée
    auto* ident = arena_.create<IdentExpr>(name);
    return arena_.create<ExprStmt>(ident);
}

Stmt* Parser::parse_if_statement() {
    Expr* cond = parse_expression();
    BlockStmt* then_b = parse_indented_block();
    auto* if_stmt = arena_.create<IfStmt>(cond, then_b);

    skip_newlines();
    while (match(TokenKind::ElifKw)) {
        Expr* elif_cond = parse_expression();
        BlockStmt* elif_b = parse_indented_block();
        if_stmt->elif_branches.emplace_back(elif_cond, elif_b);
        skip_newlines();
    }

    if (match(TokenKind::ElseKw)) {
        if_stmt->else_block = parse_indented_block();
    }

    return if_stmt;
}

Stmt* Parser::parse_while_statement() {
    Expr* cond = parse_expression();
    BlockStmt* body = parse_indented_block();
    return arena_.create<WhileStmt>(cond, body);
}

Stmt* Parser::parse_match_statement() {
    Expr* expr = parse_expression();
    consume(TokenKind::Colon, "':' attendu après l'expression de match");
    skip_newlines();
    consume(TokenKind::Indent, "Indentation attendue pour les branches de match");

    auto* match_stmt = arena_.create<MatchStmt>(expr);
    skip_newlines();

    while (!check(TokenKind::Dedent) && !check(TokenKind::Eof)) {
        if (match(TokenKind::CaseKw)) {
            Expr* pattern = parse_expression();
            BlockStmt* case_body = parse_indented_block();
            match_stmt->cases.push_back(MatchCase{pattern, case_body});
        } else if (match(TokenKind::DefaultKw)) {
            match_stmt->default_block = parse_indented_block();
        } else {
            throw std::runtime_error("'case' ou 'default' attendu dans le bloc match");
        }
        skip_newlines();
    }

    consume(TokenKind::Dedent, "Dédentation attendue à la fin du match");
    return match_stmt;
}

Stmt* Parser::parse_try_fallback_statement() {
    BlockStmt* try_b = parse_indented_block();
    skip_newlines();
    consume(TokenKind::FallbackKw, "'fallback' attendu après le bloc try");
    BlockStmt* fallback_b = parse_indented_block();
    return arena_.create<TryFallbackStmt>(try_b, fallback_b);
}

Stmt* Parser::parse_function_declaration(std::string name) {
    consume(TokenKind::LParen, "'(' attendu après le nom de fonction");
    std::vector<ParamDecl> params;

    if (!check(TokenKind::RParen)) {
        do {
            std::string p_name = consume(TokenKind::Identifier, "Nom de paramètre attendu").text;
            std::optional<OnyxType> p_type;
            if (match(TokenKind::Colon)) {
                p_type = parse_type();
            }
            params.push_back(ParamDecl{p_name, p_type});
        } while (match(TokenKind::Comma));
    }
    consume(TokenKind::RParen, "')' attendu après les paramètres");

    consume(TokenKind::ArrowThin, "'->' attendu avant le type de retour");
    OnyxType ret_type = parse_type();

    BlockStmt* body = parse_indented_block();
    return arena_.create<FunctionDeclStmt>(std::move(name), std::move(params), ret_type, body);
}

OnyxType Parser::parse_type() {
    OnyxType t;
    if (match(TokenKind::ArrKw)) {
        t.kind = TypeKind::Tensor;
        consume(TokenKind::LBracket, "'[' attendu après arr");
        std::string elem_type_name = consume(TokenKind::Identifier, "Type d'élément attendu pour arr").text;
        consume(TokenKind::RBracket, "']' attendu");

        if (elem_type_name == "int" || elem_type_name == "int32" || elem_type_name == "int64") t.element_type = TypeKind::Int;
        else if (elem_type_name == "real" || elem_type_name == "r32" || elem_type_name == "r64" || elem_type_name == "r128") t.element_type = TypeKind::Real;
        else if (elem_type_name == "complex") t.element_type = TypeKind::Complex;
        else if (elem_type_name == "quaternion") t.element_type = TypeKind::Quaternion;
        else if (elem_type_name == "bool") t.element_type = TypeKind::Bool;
        else t.element_type = TypeKind::Real;

        if (match(TokenKind::LAngle)) {
            // Lecture des dimensions <d1, d2, ...>
            do {
                if (check(TokenKind::IntLiteral)) {
                    t.shape.push_back(advance().int_val);
                } else if (check(TokenKind::Identifier)) {
                    // Dimension symbolique (ex: <n>)
                    advance();
                    t.shape.push_back(-1); // Dynamique
                }
            } while (match(TokenKind::Comma));
            consume(TokenKind::RAngle, "'>' attendu après les dimensions");
        }
        return t;
    }

    std::string type_name = consume(TokenKind::Identifier, "Nom de type attendu").text;
    if (type_name == "int" || type_name == "int32" || type_name == "int64") t.kind = TypeKind::Int;
    else if (type_name == "real" || type_name == "r32" || type_name == "r64" || type_name == "r128") t.kind = TypeKind::Real;
    else if (type_name == "complex") t.kind = TypeKind::Complex;
    else if (type_name == "quaternion") t.kind = TypeKind::Quaternion;
    else if (type_name == "bool") t.kind = TypeKind::Bool;
    else if (type_name == "string") t.kind = TypeKind::String;
    else if (type_name == "none") t.kind = TypeKind::None;
    else {
        t.kind = TypeKind::Custom;
        t.name = type_name;
    }
    return t;
}

// --- Pratt Parsing d'Expressions ---

int Parser::get_infix_bp(TokenKind kind) const noexcept {
    switch (kind) {
        case TokenKind::OrKw: return 10;
        case TokenKind::AndKw: return 20;
        case TokenKind::EqualEqual:
        case TokenKind::NotEqual: return 40;
        case TokenKind::LAngle:
        case TokenKind::RAngle:
        case TokenKind::LessEqual:
        case TokenKind::GreaterEqual: return 50;
        case TokenKind::Plus:
        case TokenKind::Minus: return 60;
        case TokenKind::Star:
        case TokenKind::Slash:
        case TokenKind::DoubleSlash:
        case TokenKind::Percent: return 70;
        case TokenKind::Caret: return 90; // Associatif à droite
        case TokenKind::LParen:
        case TokenKind::LBracket: return 100; // Appel ou Indexation
        default: return 0;
    }
}

Expr* Parser::parse_expression(int min_bp) {
    Expr* left = parse_prefix();

    while (true) {
        TokenKind op = peek().kind;
        int bp = get_infix_bp(op);
        if (bp == 0 || bp < min_bp) {
            break;
        }
        left = parse_infix(left, bp);
    }

    return left;
}

Expr* Parser::parse_prefix() {
    Token tok = advance();

    switch (tok.kind) {
        case TokenKind::IntLiteral: {
            auto* lit = arena_.create<LiteralExpr>(TypeKind::Int);
            lit->int_val = tok.int_val;
            return lit;
        }
        case TokenKind::RealLiteral: {
            auto* lit = arena_.create<LiteralExpr>(TypeKind::Real);
            lit->real_val = tok.real_val;
            return lit;
        }
        case TokenKind::ComplexLiteral: {
            auto* lit = arena_.create<LiteralExpr>(TypeKind::Complex);
            lit->real_val = tok.real_val;
            lit->imag_val = tok.imag_val;
            return lit;
        }
        case TokenKind::StringLiteral: {
            auto* lit = arena_.create<LiteralExpr>(TypeKind::String);
            lit->str_val = tok.text;
            return lit;
        }
        case TokenKind::TrueKw: {
            auto* lit = arena_.create<LiteralExpr>(TypeKind::Bool);
            lit->bool_val = true;
            return lit;
        }
        case TokenKind::FalseKw: {
            auto* lit = arena_.create<LiteralExpr>(TypeKind::Bool);
            lit->bool_val = false;
            return lit;
        }
        case TokenKind::NoneKw: {
            return arena_.create<LiteralExpr>(TypeKind::None);
        }
        case TokenKind::Identifier: {
            return arena_.create<IdentExpr>(tok.text);
        }
        case TokenKind::Plus:
        case TokenKind::Minus: {
            Expr* operand = parse_expression(80); // Priorité unaire
            return arena_.create<UnaryExpr>(tok.kind, operand);
        }
        case TokenKind::NotKw: {
            Expr* operand = parse_expression(30);
            return arena_.create<UnaryExpr>(TokenKind::NotKw, operand);
        }
        case TokenKind::CopyKw: {
            consume(TokenKind::LParen, "'(' attendu après copy");
            Expr* operand = parse_expression();
            consume(TokenKind::RParen, "')' attendu");
            return arena_.create<CopyExpr>(operand);
        }
        case TokenKind::LambdaKw: {
            // lambda (params) = body
            consume(TokenKind::LParen, "'(' attendu après lambda");
            std::vector<ParamDecl> params;
            if (!check(TokenKind::RParen)) {
                do {
                    std::string p_name = consume(TokenKind::Identifier, "Nom de paramètre lambda").text;
                    std::optional<OnyxType> p_type;
                    if (match(TokenKind::Colon)) {
                        p_type = parse_type();
                    }
                    params.push_back(ParamDecl{p_name, p_type});
                } while (match(TokenKind::Comma));
            }
            consume(TokenKind::RParen, "')' attendu après paramètres lambda");
            consume(TokenKind::Assign, "'=' attendu avant le corps de lambda");
            Expr* body = parse_expression();
            return arena_.create<LambdaExpr>(std::move(params), body);
        }
        case TokenKind::LBracket: {
            // Littéral de tenseur : [1, 2, 3]
            std::vector<Expr*> elements;
            if (!check(TokenKind::RBracket)) {
                do {
                    elements.push_back(parse_expression());
                } while (match(TokenKind::Comma));
            }
            consume(TokenKind::RBracket, "']' attendu");
            return arena_.create<TensorLiteralExpr>(std::move(elements));
        }
        case TokenKind::LParen: {
            // Groupement (expr)
            Expr* expr = parse_expression();
            consume(TokenKind::RParen, "')' fermante attendue");
            return expr;
        }
        case TokenKind::QuaternionKw: {
            // quaternion(r, i, j, k)
            consume(TokenKind::LParen, "'(' attendu après quaternion");
            std::vector<Expr*> args;
            do {
                args.push_back(parse_expression());
            } while (match(TokenKind::Comma));
            consume(TokenKind::RParen, "')' attendu");
            return arena_.create<CallExpr>("quaternion", std::move(args));
        }
        case TokenKind::AbsKw:
        case TokenKind::ExpKw:
        case TokenKind::LnKw:
        case TokenKind::PrintKw: {
            std::string func_name = tok.text;
            consume(TokenKind::LParen, "'(' attendu");
            std::vector<Expr*> args;
            if (!check(TokenKind::RParen)) {
                do {
                    args.push_back(parse_expression());
                } while (match(TokenKind::Comma));
            }
            consume(TokenKind::RParen, "')' attendu");
            return arena_.create<CallExpr>(std::move(func_name), std::move(args));
        }
        default:
            throw std::runtime_error(std::string("Expression inattendue : ") + token_kind_to_string(tok.kind) +
                                     " à la ligne " + std::to_string(tok.span.line));
    }
}

Expr* Parser::parse_infix(Expr* left, int min_bp) {
    Token op = advance();

    // Appel de fonction : expr(arg1, arg2)
    if (op.kind == TokenKind::LParen) {
        std::vector<Expr*> args;
        if (!check(TokenKind::RParen)) {
            do {
                args.push_back(parse_expression());
            } while (match(TokenKind::Comma));
        }
        consume(TokenKind::RParen, "')' attendu après les arguments d'appel");
        if (left->kind == ExprKind::Identifier) {
            return arena_.create<CallExpr>(static_cast<IdentExpr*>(left)->name, std::move(args));
        }
        throw std::runtime_error("Appel non supporté sur expression non-identifiant");
    }

    // Indexation de tenseur : tensor[i, j, k]
    if (op.kind == TokenKind::LBracket) {
        std::vector<Expr*> indices;
        do {
            indices.push_back(parse_expression());
        } while (match(TokenKind::Comma));
        consume(TokenKind::RBracket, "']' attendu");
        return arena_.create<TensorIndexExpr>(left, std::move(indices));
    }

    // Puissance ^ (associativité droite : min_bp - 1)
    int next_bp = (op.kind == TokenKind::Caret) ? (min_bp - 1) : min_bp;
    Expr* right = parse_expression(next_bp);
    return arena_.create<BinaryExpr>(op.kind, left, right);
}

} // namespace onyx
