#include "linear_check.hpp"

namespace onyx {

LinearChecker::LinearChecker() {
    enter_scope(); // Scope global
}

void LinearChecker::enter_scope() {
    scopes_.emplace_back();
}

void LinearChecker::exit_scope() {
    if (!scopes_.empty()) {
        scopes_.pop_back();
    }
}

void LinearChecker::declare_variable(const std::string& name, bool is_persistent, bool is_linear) {
    if (scopes_.empty()) enter_scope();
    VarInfo info;
    info.is_persistent = is_persistent;
    info.is_linear = is_linear;
    info.state = LinearState::Valid;
    scopes_.back()[name] = info;
}

VarInfo* LinearChecker::lookup_variable(const std::string& name) {
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) {
            return &found->second;
        }
    }
    return nullptr;
}

void LinearChecker::check_program(const BlockStmt* root) {
    if (!root) return;
    for (const auto* s : root->statements) {
        check_stmt(s);
    }
}

void LinearChecker::check_stmt(const Stmt* stmt) {
    if (!stmt) return;

    switch (stmt->kind) {
        case StmtKind::VarDecl: {
            const auto* v = static_cast<const VarDeclStmt*>(stmt);
            bool is_linear = false;
            if (v->type.has_value() && v->type->kind == TypeKind::Tensor) {
                is_linear = true;
            } else if (v->init_expr && v->init_expr->kind == ExprKind::TensorLiteral) {
                is_linear = true;
            } else if (v->init_expr && v->init_expr->kind == ExprKind::Identifier) {
                // Hérite de la linéarité de la variable source
                const auto* src_ident = static_cast<const IdentExpr*>(v->init_expr);
                VarInfo* src_var = lookup_variable(src_ident->name);
                if (src_var && src_var->is_linear) {
                    is_linear = true;
                }
            }

            if (v->init_expr) {
                // Dans une assignation directe b = a, si c'est linéaire, 'a' est consommé
                check_expr(v->init_expr, is_linear);
            }

            declare_variable(v->name, v->is_persistent, is_linear);
            break;
        }
        case StmtKind::Assign: {
            const auto* a = static_cast<const AssignStmt*>(stmt);
            VarInfo* target = lookup_variable(a->target_name);
            bool is_linear = target ? target->is_linear : false;

            if (a->value_expr) {
                check_expr(a->value_expr, is_linear);
            }
            break;
        }
        case StmtKind::TensorAssign: {
            const auto* ta = static_cast<const TensorAssignStmt*>(stmt);
            VarInfo* t = lookup_variable(ta->tensor_name);
            if (t && t->state == LinearState::Consumed) {
                throw std::runtime_error("Erreur de linéarité : le tenseur '" + ta->tensor_name +
                                         "' a été consommé et ne peut plus être modifié");
            }
            for (const auto* idx : ta->indices) {
                check_expr(idx, false);
            }
            check_expr(ta->value_expr, false);
            break;
        }
        case StmtKind::Expr: {
            const auto* es = static_cast<const ExprStmt*>(stmt);
            check_expr(es->expr, false);
            break;
        }
        case StmtKind::Return: {
            const auto* rs = static_cast<const ReturnStmt*>(stmt);
            if (rs->value_expr) {
                check_expr(rs->value_expr, true); // Le retour transfère l'ownership
            }
            break;
        }
        case StmtKind::If: {
            const auto* is = static_cast<const IfStmt*>(stmt);
            check_expr(is->condition, false);
            enter_scope();
            check_stmt(is->then_block);
            exit_scope();

            for (const auto& branch : is->elif_branches) {
                check_expr(branch.first, false);
                enter_scope();
                check_stmt(branch.second);
                exit_scope();
            }

            if (is->else_block) {
                enter_scope();
                check_stmt(is->else_block);
                exit_scope();
            }
            break;
        }
        case StmtKind::While: {
            const auto* ws = static_cast<const WhileStmt*>(stmt);
            check_expr(ws->condition, false);
            enter_scope();
            check_stmt(ws->body);
            exit_scope();
            break;
        }
        case StmtKind::FunctionDecl: {
            const auto* fs = static_cast<const FunctionDeclStmt*>(stmt);
            enter_scope();
            for (const auto& p : fs->params) {
                bool is_lin = p.type.has_value() && p.type->kind == TypeKind::Tensor;
                declare_variable(p.name, false, is_lin);
            }
            check_stmt(fs->body);
            exit_scope();
            break;
        }
        case StmtKind::Block: {
            const auto* bs = static_cast<const BlockStmt*>(stmt);
            for (const auto* s : bs->statements) {
                check_stmt(s);
            }
            break;
        }
        default:
            break;
    }
}

void LinearChecker::check_expr(const Expr* expr, bool is_consuming_context) {
    if (!expr) return;

    switch (expr->kind) {
        case ExprKind::Identifier: {
            const auto* id = static_cast<const IdentExpr*>(expr);
            VarInfo* v = lookup_variable(id->name);
            if (v) {
                if (v->state == LinearState::Consumed) {
                    throw std::runtime_error("Erreur de linéarité : la variable '" + id->name +
                                             "' a été consommée par une assignation précédente");
                }
                if (is_consuming_context && v->is_linear && !v->is_persistent) {
                    // Consommation de la variable linéaire !
                    v->state = LinearState::Consumed;
                }
            }
            break;
        }
        case ExprKind::Unary: {
            const auto* u = static_cast<const UnaryExpr*>(expr);
            check_expr(u->operand, false);
            break;
        }
        case ExprKind::Binary: {
            const auto* b = static_cast<const BinaryExpr*>(expr);
            check_expr(b->left, false);
            check_expr(b->right, false);
            break;
        }
        case ExprKind::Call: {
            const auto* c = static_cast<const CallExpr*>(expr);
            for (const auto* arg : c->args) {
                // print(a) ou sum(a) ne consomment pas, mais une fonction personnalisée peut consommer
                bool consumes_arg = (c->callee != "print" && c->callee != "sum" &&
                                     c->callee != "first" && c->callee != "product" &&
                                     c->callee != "abs" && c->callee != "exp" && c->callee != "ln");
                check_expr(arg, consumes_arg);
            }
            break;
        }
        case ExprKind::TensorIndex: {
            const auto* ti = static_cast<const TensorIndexExpr*>(expr);
            check_expr(ti->tensor, false); // Indexation ne consomme pas le tenseur
            for (const auto* idx : ti->indices) {
                check_expr(idx, false);
            }
            break;
        }
        case ExprKind::Copy: {
            const auto* cp = static_cast<const CopyExpr*>(expr);
            check_expr(cp->operand, false); // copy() empêche la consommation de l'opérande
            break;
        }
        default:
            break;
    }
}

} // namespace onyx
