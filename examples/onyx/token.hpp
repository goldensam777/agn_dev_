#pragma once

#include <string>
#include <string_view>
#include <cstdint>

namespace onyx {

enum class TokenKind {
    // Fin de fichier et formatage d'indentation
    Eof,
    Newline,
    Indent,
    Dedent,

    // Littéraux
    IntLiteral,
    RealLiteral,
    ComplexLiteral,
    StringLiteral,
    Identifier,

    // Modificateurs & Directives Spéciales
    BangI,        // !i (variable persistante / non-consommée)
    HashFrom,     // #from
    HashParents,  // #parents
    HashFormat,   // #format
    BreakKw,      // :break

    // Mots-clés de flux et déclarations
    IfKw,
    ElifKw,
    ElseKw,
    WhileKw,
    MatchKw,
    CaseKw,
    DefaultKw,
    TryKw,
    FallbackKw,
    LambdaKw,
    PrintKw,
    NoneKw,
    TrueKw,
    FalseKw,
    AndKw,
    OrKw,
    NotKw,
    CopyKw,
    ArrKw,
    ClassKw,
    QuaternionKw,
    AbsKw,
    ExpKw,
    LnKw,

    // Ponctuation et Délimiteurs
    Colon,        // :
    ArrowFat,     // => (retour de fonction immédiat)
    ArrowThin,    // -> (type de retour)
    LParen,       // (
    RParen,       // )
    LBracket,     // [
    RBracket,     // ]
    LAngle,       // <
    RAngle,       // >
    Comma,        // ,
    Assign,       // =
    Dot,          // .

    // Opérateurs arithmétiques et d'assignation
    Plus,         // +
    Minus,        // -
    Star,         // *
    Slash,        // /
    DoubleSlash,  // // (division entière)
    Percent,      // % (modulo)
    Caret,        // ^ (puissance / exponentiation)
    PlusAssign,   // +=
    MinusAssign,  // -=

    // Comparaisons
    EqualEqual,   // ==
    NotEqual,     // !=
    LessEqual,    // <=
    GreaterEqual  // >=
};

struct SourceSpan {
    size_t line{1};
    size_t column{1};
    size_t length{0};
};

struct Token {
    TokenKind kind{TokenKind::Eof};
    std::string text;
    int64_t int_val{0};
    double real_val{0.0};
    double imag_val{0.0};
    SourceSpan span;
};

[[nodiscard]] inline const char* token_kind_to_string(TokenKind kind) noexcept {
    switch (kind) {
        case TokenKind::Eof: return "EOF";
        case TokenKind::Newline: return "NEWLINE";
        case TokenKind::Indent: return "INDENT";
        case TokenKind::Dedent: return "DEDENT";
        case TokenKind::IntLiteral: return "INT";
        case TokenKind::RealLiteral: return "REAL";
        case TokenKind::ComplexLiteral: return "COMPLEX";
        case TokenKind::StringLiteral: return "STRING";
        case TokenKind::Identifier: return "IDENTIFIER";
        case TokenKind::BangI: return "!i";
        case TokenKind::HashFrom: return "#from";
        case TokenKind::HashParents: return "#parents";
        case TokenKind::HashFormat: return "#format";
        case TokenKind::BreakKw: return ":break";
        case TokenKind::IfKw: return "if";
        case TokenKind::ElifKw: return "elif";
        case TokenKind::ElseKw: return "else";
        case TokenKind::WhileKw: return "while";
        case TokenKind::MatchKw: return "match";
        case TokenKind::CaseKw: return "case";
        case TokenKind::DefaultKw: return "default";
        case TokenKind::TryKw: return "try";
        case TokenKind::FallbackKw: return "fallback";
        case TokenKind::LambdaKw: return "lambda";
        case TokenKind::PrintKw: return "print";
        case TokenKind::NoneKw: return "none";
        case TokenKind::TrueKw: return "true";
        case TokenKind::FalseKw: return "false";
        case TokenKind::AndKw: return "and";
        case TokenKind::OrKw: return "or";
        case TokenKind::NotKw: return "not";
        case TokenKind::CopyKw: return "copy";
        case TokenKind::ArrKw: return "arr";
        case TokenKind::ClassKw: return "class";
        case TokenKind::QuaternionKw: return "quaternion";
        case TokenKind::AbsKw: return "abs";
        case TokenKind::ExpKw: return "exp";
        case TokenKind::LnKw: return "ln";
        case TokenKind::Colon: return ":";
        case TokenKind::ArrowFat: return "=>";
        case TokenKind::ArrowThin: return "->";
        case TokenKind::LParen: return "(";
        case TokenKind::RParen: return ")";
        case TokenKind::LBracket: return "[";
        case TokenKind::RBracket: return "]";
        case TokenKind::LAngle: return "<";
        case TokenKind::RAngle: return ">";
        case TokenKind::Comma: return ",";
        case TokenKind::Assign: return "=";
        case TokenKind::Dot: return ".";
        case TokenKind::Plus: return "+";
        case TokenKind::Minus: return "-";
        case TokenKind::Star: return "*";
        case TokenKind::Slash: return "/";
        case TokenKind::DoubleSlash: return "//";
        case TokenKind::Percent: return "%";
        case TokenKind::Caret: return "^";
        case TokenKind::PlusAssign: return "+=";
        case TokenKind::MinusAssign: return "-=";
        case TokenKind::EqualEqual: return "==";
        case TokenKind::NotEqual: return "!=";
        case TokenKind::LessEqual: return "<=";
        case TokenKind::GreaterEqual: return ">=";
    }
    return "UNKNOWN";
}

} // namespace onyx
