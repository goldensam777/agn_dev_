#include "lexer.hpp"
#include <cctype>
#include <stdexcept>

namespace onyx {

Lexer::Lexer(std::string_view source)
    : source_(source), cursor_(0), line_(1), column_(1), indent_stack_{0}, pending_dedents_(0), at_line_start_(true) {}

char Lexer::peek() const noexcept {
    if (is_at_end()) return '\0';
    return source_[cursor_];
}

char Lexer::peek_next() const noexcept {
    if (cursor_ + 1 >= source_.length()) return '\0';
    return source_[cursor_ + 1];
}

char Lexer::advance() noexcept {
    if (is_at_end()) return '\0';
    char c = source_[cursor_++];
    column_++;
    return c;
}

bool Lexer::match(char expected) noexcept {
    if (is_at_end() || source_[cursor_] != expected) return false;
    cursor_++;
    column_++;
    return true;
}

bool Lexer::is_at_end() const noexcept {
    return cursor_ >= source_.length();
}

void Lexer::skip_comment() {
    // (< commentaire >)
    advance(); // consomme '('
    advance(); // consomme '<'

    int depth = 1;
    while (!is_at_end() && depth > 0) {
        if (peek() == '\n') {
            line_++;
            column_ = 0;
            advance();
        } else if (peek() == '(' && peek_next() == '<') {
            depth++;
            advance();
            advance();
        } else if (peek() == '>' && peek_next() == ')') {
            depth--;
            advance();
            advance();
        } else {
            advance();
        }
    }
}

void Lexer::skip_whitespace_and_comments() {
    while (!is_at_end()) {
        char c = peek();
        if (c == ' ' || c == '\t' || c == '\r') {
            advance();
        } else if (c == '(' && peek_next() == '<') {
            skip_comment();
        } else {
            break;
        }
    }
}

Token Lexer::make_token(TokenKind kind, std::string text) {
    Token tok;
    tok.kind = kind;
    tok.text = std::move(text);
    tok.span.line = line_;
    tok.span.column = column_;
    tok.span.length = tok.text.length();
    return tok;
}

Token Lexer::next_token() {
    // Si des DEDENT sont en attente
    if (pending_dedents_ > 0) {
        pending_dedents_--;
        return make_token(TokenKind::Dedent, "");
    }

    if (at_line_start_) {
        // Calcul de l'indentation de début de ligne
        size_t current_indent = 0;
        size_t temp_cursor = cursor_;
        size_t temp_line = line_;

        while (temp_cursor < source_.length()) {
            char c = source_[temp_cursor];
            if (c == ' ') {
                current_indent++;
                temp_cursor++;
            } else if (c == '\t') {
                current_indent += 4;
                temp_cursor++;
            } else if (c == '(' && temp_cursor + 1 < source_.length() && source_[temp_cursor + 1] == '<') {
                // Commentaire en début de ligne : on saute
                cursor_ = temp_cursor;
                skip_comment();
                temp_cursor = cursor_;
            } else if (c == '\r') {
                temp_cursor++;
            } else if (c == '\n') {
                // Ligne vide : ignorer et passer à la ligne suivante
                temp_cursor++;
                temp_line++;
                current_indent = 0;
            } else {
                break;
            }
        }

        if (temp_cursor >= source_.length()) {
            cursor_ = temp_cursor;
            line_ = temp_line;
            // Émission des DEDENT finaux pour vider la pile jusqu'à 0
            if (indent_stack_.size() > 1) {
                indent_stack_.pop_back();
                return make_token(TokenKind::Dedent, "");
            }
            return make_token(TokenKind::Eof, "");
        }

        cursor_ = temp_cursor;
        line_ = temp_line;
        column_ = current_indent + 1;
        at_line_start_ = false;

        size_t previous_indent = indent_stack_.back();
        if (current_indent > previous_indent) {
            indent_stack_.push_back(current_indent);
            return make_token(TokenKind::Indent, "");
        } else if (current_indent < previous_indent) {
            while (indent_stack_.size() > 1 && indent_stack_.back() > current_indent) {
                indent_stack_.pop_back();
                pending_dedents_++;
            }
            if (indent_stack_.back() != current_indent) {
                throw std::runtime_error("Erreur d'indentation à la ligne " + std::to_string(line_));
            }
            pending_dedents_--;
            return make_token(TokenKind::Dedent, "");
        }
    }

    skip_whitespace_and_comments();

    if (is_at_end()) {
        if (indent_stack_.size() > 1) {
            indent_stack_.pop_back();
            return make_token(TokenKind::Dedent, "");
        }
        return make_token(TokenKind::Eof, "");
    }

    char c = peek();

    if (c == '\n') {
        advance();
        line_++;
        column_ = 1;
        at_line_start_ = true;
        return make_token(TokenKind::Newline, "\n");
    }

    // Nombre (int, real, complex)
    if (std::isdigit(static_cast<unsigned char>(c))) {
        return number();
    }

    // Chaîne de caractères
    if (c == '"' || c == '\'') {
        return string(c);
    }

    // Modificateurs spéciaux et préfixes
    if (c == '!' && peek_next() == 'i') {
        advance(); // !
        advance(); // i
        return make_token(TokenKind::BangI, "!i");
    }

    if (c == '#') {
        advance(); // #
        std::string tag;
        while (std::isalpha(static_cast<unsigned char>(peek()))) {
            tag += advance();
        }
        if (tag == "from") return make_token(TokenKind::HashFrom, "#from");
        if (tag == "parents") return make_token(TokenKind::HashParents, "#parents");
        if (tag == "format") return make_token(TokenKind::HashFormat, "#format");
        return make_token(TokenKind::Identifier, "#" + tag);
    }

    if (c == ':' && peek_next() == 'b') {
        // Vérifier si c'est :break
        size_t save_cur = cursor_;
        advance(); // :
        std::string word;
        while (std::isalpha(static_cast<unsigned char>(peek()))) {
            word += advance();
        }
        if (word == "break") {
            return make_token(TokenKind::BreakKw, ":break");
        }
        // Sinon restaurer et retourner le colon simple
        cursor_ = save_cur;
    }

    // Identifiant ou mot-clé
    if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
        return identifier_or_keyword();
    }

    // Opérateurs et ponctuation
    advance();
    switch (c) {
        case '(': return make_token(TokenKind::LParen, "(");
        case ')': return make_token(TokenKind::RParen, ")");
        case '[': return make_token(TokenKind::LBracket, "[");
        case ']': return make_token(TokenKind::RBracket, "]");
        case '<':
            if (match('=')) return make_token(TokenKind::LessEqual, "<=");
            return make_token(TokenKind::LAngle, "<");
        case '>':
            if (match('=')) return make_token(TokenKind::GreaterEqual, ">=");
            return make_token(TokenKind::RAngle, ">");
        case ',': return make_token(TokenKind::Comma, ",");
        case '.': return make_token(TokenKind::Dot, ".");
        case ':': return make_token(TokenKind::Colon, ":");
        case '^': return make_token(TokenKind::Caret, "^");
        case '%': return make_token(TokenKind::Percent, "%");
        case '=':
            if (match('>')) return make_token(TokenKind::ArrowFat, "=>");
            if (match('=')) return make_token(TokenKind::EqualEqual, "==");
            return make_token(TokenKind::Assign, "=");
        case '!':
            if (match('=')) return make_token(TokenKind::NotEqual, "!=");
            break;
        case '+':
            if (match('=')) return make_token(TokenKind::PlusAssign, "+=");
            return make_token(TokenKind::Plus, "+");
        case '-':
            if (match('>')) return make_token(TokenKind::ArrowThin, "->");
            if (match('=')) return make_token(TokenKind::MinusAssign, "-=");
            return make_token(TokenKind::Minus, "-");
        case '*':
            return make_token(TokenKind::Star, "*");
        case '/':
            if (match('/')) return make_token(TokenKind::DoubleSlash, "//");
            return make_token(TokenKind::Slash, "/");
    }

    throw std::runtime_error(std::string("Caractère inattendu : '") + c + "' à la ligne " + std::to_string(line_));
}

Token Lexer::number() {
    std::string text;
    bool is_real = false;

    while (std::isdigit(static_cast<unsigned char>(peek()))) {
        text += advance();
    }

    if (peek() == '.' && std::isdigit(static_cast<unsigned char>(peek_next()))) {
        is_real = true;
        text += advance(); // .
        while (std::isdigit(static_cast<unsigned char>(peek()))) {
            text += advance();
        }
    }

    // Gestion de l'unité imaginaire pour les complexes (ex: 52i, 2.5i)
    if (peek() == 'i' && !std::isalnum(static_cast<unsigned char>(peek_next()))) {
        advance(); // consomme 'i'
        Token tok = make_token(TokenKind::ComplexLiteral, text + "i");
        tok.real_val = 0.0;
        tok.imag_val = std::stod(text);
        return tok;
    }

    if (is_real) {
        Token tok = make_token(TokenKind::RealLiteral, text);
        tok.real_val = std::stod(text);
        return tok;
    } else {
        Token tok = make_token(TokenKind::IntLiteral, text);
        tok.int_val = std::stoll(text);
        return tok;
    }
}

Token Lexer::string(char quote_char) {
    advance(); // Consomme le guillemet initial
    std::string text;
    while (!is_at_end() && peek() != quote_char) {
        if (peek() == '\n') {
            line_++;
            column_ = 1;
        }
        text += advance();
    }
    if (!is_at_end()) {
        advance(); // Consomme le guillemet fermant
    }
    return make_token(TokenKind::StringLiteral, text);
}

Token Lexer::identifier_or_keyword() {
    std::string text;
    while (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_') {
        text += advance();
    }

    // Mots-clés
    if (text == "if") return make_token(TokenKind::IfKw, text);
    if (text == "elif") return make_token(TokenKind::ElifKw, text);
    if (text == "else") return make_token(TokenKind::ElseKw, text);
    if (text == "while") return make_token(TokenKind::WhileKw, text);
    if (text == "match") return make_token(TokenKind::MatchKw, text);
    if (text == "case") return make_token(TokenKind::CaseKw, text);
    if (text == "default") return make_token(TokenKind::DefaultKw, text);
    if (text == "try") return make_token(TokenKind::TryKw, text);
    if (text == "fallback") return make_token(TokenKind::FallbackKw, text);
    if (text == "lambda") return make_token(TokenKind::LambdaKw, text);
    if (text == "print") return make_token(TokenKind::PrintKw, text);
    if (text == "none") return make_token(TokenKind::NoneKw, text);
    if (text == "true") return make_token(TokenKind::TrueKw, text);
    if (text == "false") return make_token(TokenKind::FalseKw, text);
    if (text == "and") return make_token(TokenKind::AndKw, text);
    if (text == "or") return make_token(TokenKind::OrKw, text);
    if (text == "not") return make_token(TokenKind::NotKw, text);
    if (text == "copy") return make_token(TokenKind::CopyKw, text);
    if (text == "arr") return make_token(TokenKind::ArrKw, text);
    if (text == "class") return make_token(TokenKind::ClassKw, text);
    if (text == "quaternion") return make_token(TokenKind::QuaternionKw, text);
    if (text == "abs") return make_token(TokenKind::AbsKw, text);
    if (text == "exp") return make_token(TokenKind::ExpKw, text);
    if (text == "ln") return make_token(TokenKind::LnKw, text);

    return make_token(TokenKind::Identifier, text);
}

std::vector<Token> Lexer::tokenize_all() {
    std::vector<Token> tokens;
    while (true) {
        Token tok = next_token();
        tokens.push_back(tok);
        if (tok.kind == TokenKind::Eof) break;
    }
    return tokens;
}

} // namespace onyx
