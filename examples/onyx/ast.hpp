#pragma once

#include "token.hpp"
#include <string>
#include <vector>
#include <memory>
#include <optional>

namespace onyx {

enum class TypeKind {
    Int,
    Real,
    Complex,
    Quaternion,
    Bool,
    String,
    None,
    Tensor,
    Custom
};

struct OnyxType {
    TypeKind kind{TypeKind::None};
    std::string name;
    // Pour les tenseurs : arr[ElementType] <d1, ..., dN>
    TypeKind element_type{TypeKind::Real};
    std::vector<int64_t> shape;
};

// --- Expressions ---

enum class ExprKind {
    Literal,
    Identifier,
    Unary,
    Binary,
    Call,
    TensorIndex,
    TensorLiteral,
    Lambda,
    Copy
};

struct Expr {
    ExprKind kind;
    SourceSpan span;
    explicit Expr(ExprKind k) : kind(k) {}
    virtual ~Expr() = default;
};

struct LiteralExpr : public Expr {
    TypeKind type_kind;
    int64_t int_val{0};
    double real_val{0.0};
    double imag_val{0.0};
    double q_r{0.0}, q_i{0.0}, q_j{0.0}, q_k{0.0};
    bool bool_val{false};
    std::string str_val;

    explicit LiteralExpr(TypeKind tk) : Expr(ExprKind::Literal), type_kind(tk) {}
};

struct IdentExpr : public Expr {
    std::string name;
    explicit IdentExpr(std::string n) : Expr(ExprKind::Identifier), name(std::move(n)) {}
};

struct UnaryExpr : public Expr {
    TokenKind op;
    Expr* operand;
    UnaryExpr(TokenKind o, Expr* opnd) : Expr(ExprKind::Unary), op(o), operand(opnd) {}
};

struct BinaryExpr : public Expr {
    TokenKind op;
    Expr* left;
    Expr* right;
    BinaryExpr(TokenKind o, Expr* l, Expr* r) : Expr(ExprKind::Binary), op(o), left(l), right(r) {}
};

struct CallExpr : public Expr {
    std::string callee;
    std::vector<Expr*> args;
    CallExpr(std::string c, std::vector<Expr*> a) : Expr(ExprKind::Call), callee(std::move(c)), args(std::move(a)) {}
};

struct TensorIndexExpr : public Expr {
    Expr* tensor;
    std::vector<Expr*> indices;
    TensorIndexExpr(Expr* t, std::vector<Expr*> idxs) : Expr(ExprKind::TensorIndex), tensor(t), indices(std::move(idxs)) {}
};

struct TensorLiteralExpr : public Expr {
    std::vector<Expr*> elements;
    explicit TensorLiteralExpr(std::vector<Expr*> elems) : Expr(ExprKind::TensorLiteral), elements(std::move(elems)) {}
};

struct ParamDecl {
    std::string name;
    std::optional<OnyxType> type;
};

struct LambdaExpr : public Expr {
    std::vector<ParamDecl> params;
    Expr* body;
    LambdaExpr(std::vector<ParamDecl> p, Expr* b) : Expr(ExprKind::Lambda), params(std::move(p)), body(b) {}
};

struct CopyExpr : public Expr {
    Expr* operand;
    explicit CopyExpr(Expr* opnd) : Expr(ExprKind::Copy), operand(opnd) {}
};

// --- Instructions (Statements) ---

enum class StmtKind {
    VarDecl,
    Assign,
    TensorAssign,
    Return,
    Break,
    Expr,
    If,
    While,
    Match,
    TryFallback,
    FunctionDecl,
    Block
};

struct Stmt {
    StmtKind kind;
    SourceSpan span;
    explicit Stmt(StmtKind k) : kind(k) {}
    virtual ~Stmt() = default;
};

struct BlockStmt : public Stmt {
    std::vector<Stmt*> statements;
    BlockStmt() : Stmt(StmtKind::Block) {}
};

struct VarDeclStmt : public Stmt {
    std::string name;
    bool is_persistent{false}; // Préfixe !i
    std::optional<OnyxType> type;
    Expr* init_expr{nullptr};

    VarDeclStmt(std::string n, bool persistent, std::optional<OnyxType> t, Expr* init)
        : Stmt(StmtKind::VarDecl), name(std::move(n)), is_persistent(persistent), type(std::move(t)), init_expr(init) {}
};

struct AssignStmt : public Stmt {
    std::string target_name;
    Expr* value_expr;
    AssignStmt(std::string name, Expr* val) : Stmt(StmtKind::Assign), target_name(std::move(name)), value_expr(val) {}
};

struct TensorAssignStmt : public Stmt {
    std::string tensor_name;
    std::vector<Expr*> indices;
    Expr* value_expr;
    TensorAssignStmt(std::string name, std::vector<Expr*> idxs, Expr* val)
        : Stmt(StmtKind::TensorAssign), tensor_name(std::move(name)), indices(std::move(idxs)), value_expr(val) {}
};

struct ReturnStmt : public Stmt {
    Expr* value_expr; // => value_expr
    explicit ReturnStmt(Expr* val) : Stmt(StmtKind::Return), value_expr(val) {}
};

struct BreakStmt : public Stmt {
    Expr* value_expr; // :break value?
    explicit BreakStmt(Expr* val = nullptr) : Stmt(StmtKind::Break), value_expr(val) {}
};

struct ExprStmt : public Stmt {
    Expr* expr;
    explicit ExprStmt(Expr* e) : Stmt(StmtKind::Expr), expr(e) {}
};

struct IfStmt : public Stmt {
    Expr* condition;
    BlockStmt* then_block;
    std::vector<std::pair<Expr*, BlockStmt*>> elif_branches;
    BlockStmt* else_block{nullptr};

    IfStmt(Expr* cond, BlockStmt* then_b) : Stmt(StmtKind::If), condition(cond), then_block(then_b) {}
};

struct WhileStmt : public Stmt {
    Expr* condition;
    BlockStmt* body;
    WhileStmt(Expr* cond, BlockStmt* b) : Stmt(StmtKind::While), condition(cond), body(b) {}
};

struct MatchCase {
    Expr* pattern;
    BlockStmt* body;
};

struct MatchStmt : public Stmt {
    Expr* expr;
    std::vector<MatchCase> cases;
    BlockStmt* default_block{nullptr};
    explicit MatchStmt(Expr* e) : Stmt(StmtKind::Match), expr(e) {}
};

struct TryFallbackStmt : public Stmt {
    BlockStmt* try_block;
    BlockStmt* fallback_block;
    TryFallbackStmt(BlockStmt* t, BlockStmt* f) : Stmt(StmtKind::TryFallback), try_block(t), fallback_block(f) {}
};

struct FunctionDeclStmt : public Stmt {
    std::string name;
    std::vector<ParamDecl> params;
    OnyxType return_type;
    BlockStmt* body;

    FunctionDeclStmt(std::string n, std::vector<ParamDecl> p, OnyxType ret, BlockStmt* b)
        : Stmt(StmtKind::FunctionDecl), name(std::move(n)), params(std::move(p)), return_type(std::move(ret)), body(b) {}
};

} // namespace onyx
