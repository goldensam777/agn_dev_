#include "runtime.hpp"
#include <cmath>
#include <iostream>
#include <sstream>

namespace onyx {

Runtime::Runtime() = default;

Value Runtime::execute(const BlockStmt* root) {
    Environment global_env;
    bool returned = false;
    Value result = Value::make_none();

    if (!root) return result;

    for (const auto* stmt : root->statements) {
        result = execute_stmt(stmt, global_env, returned);
        if (returned) break;
    }
    return result;
}

Value Runtime::execute_stmt(const Stmt* stmt, Environment& env, bool& returned) {
    if (!stmt) return Value::make_none();

    switch (stmt->kind) {
        case StmtKind::VarDecl: {
            const auto* v = static_cast<const VarDeclStmt*>(stmt);
            Value val = Value::make_none();
            if (v->init_expr) {
                val = evaluate(v->init_expr, env);
            } else if (v->type.has_value() && v->type->kind == TypeKind::Tensor) {
                // Alloue un tenseur vide selon la forme
                val = Value::make_tensor(Tensor(v->type->shape, 0.0));
            }
            env.set(v->name, std::move(val));
            return Value::make_none();
        }
        case StmtKind::Assign: {
            const auto* a = static_cast<const AssignStmt*>(stmt);
            Value val = evaluate(a->value_expr, env);
            env.set(a->target_name, std::move(val));
            return Value::make_none();
        }
        case StmtKind::TensorAssign: {
            const auto* ta = static_cast<const TensorAssignStmt*>(stmt);
            Value* t_val = env.get(ta->tensor_name);
            if (!t_val || t_val->kind != ValueKind::Tensor) {
                throw std::runtime_error("Tenseur non trouvé pour assignation : " + ta->tensor_name);
            }
            std::vector<int64_t> idxs;
            for (const auto* ie : ta->indices) {
                Value iv = evaluate(ie, env);
                if (iv.kind != ValueKind::Int) {
                    throw std::runtime_error("Les indices de tenseur doivent être des entiers");
                }
                idxs.push_back(iv.int_val);
            }
            Value val = evaluate(ta->value_expr, env);
            t_val->tensor_val.set(idxs, to_real(val));
            return Value::make_none();
        }
        case StmtKind::Return: {
            const auto* r = static_cast<const ReturnStmt*>(stmt);
            returned = true;
            if (r->value_expr) {
                return evaluate(r->value_expr, env);
            }
            return Value::make_none();
        }
        case StmtKind::Expr: {
            const auto* es = static_cast<const ExprStmt*>(stmt);
            return evaluate(es->expr, env);
        }
        case StmtKind::If: {
            const auto* is = static_cast<const IfStmt*>(stmt);
            Value cond = evaluate(is->condition, env);
            if (cond.kind != ValueKind::Bool) {
                throw std::runtime_error("La condition d'un 'if' doit être un booléen strict");
            }
            if (cond.bool_val) {
                for (const auto* s : is->then_block->statements) {
                    Value r = execute_stmt(s, env, returned);
                    if (returned) return r;
                }
                return Value::make_none();
            }

            for (const auto& elif_b : is->elif_branches) {
                Value e_cond = evaluate(elif_b.first, env);
                if (e_cond.kind != ValueKind::Bool) {
                    throw std::runtime_error("La condition d'un 'elif' doit être un booléen strict");
                }
                if (e_cond.bool_val) {
                    for (const auto* s : elif_b.second->statements) {
                        Value r = execute_stmt(s, env, returned);
                        if (returned) return r;
                    }
                    return Value::make_none();
                }
            }

            if (is->else_block) {
                for (const auto* s : is->else_block->statements) {
                    Value r = execute_stmt(s, env, returned);
                    if (returned) return r;
                }
            }
            return Value::make_none();
        }
        case StmtKind::While: {
            const auto* ws = static_cast<const WhileStmt*>(stmt);
            while (true) {
                Value cond = evaluate(ws->condition, env);
                if (cond.kind != ValueKind::Bool) {
                    throw std::runtime_error("La condition d'un 'while' doit être un booléen strict");
                }
                if (!cond.bool_val) break;

                bool break_hit = false;
                for (const auto* s : ws->body->statements) {
                    if (s->kind == StmtKind::Break) {
                        break_hit = true;
                        break;
                    }
                    Value r = execute_stmt(s, env, returned);
                    if (returned) return r;
                }
                if (break_hit) break;
            }
            return Value::make_none();
        }
        case StmtKind::Match: {
            const auto* ms = static_cast<const MatchStmt*>(stmt);
            Value target = evaluate(ms->expr, env);

            bool matched = false;
            for (const auto& mc : ms->cases) {
                Value pat = evaluate(mc.pattern, env);
                Value cmp = apply_binary_op(TokenKind::EqualEqual, target, pat);
                if (cmp.bool_val) {
                    matched = true;
                    for (const auto* s : mc.body->statements) {
                        Value r = execute_stmt(s, env, returned);
                        if (returned) return r;
                    }
                    break;
                }
            }
            if (!matched && ms->default_block) {
                for (const auto* s : ms->default_block->statements) {
                    Value r = execute_stmt(s, env, returned);
                    if (returned) return r;
                }
            }
            return Value::make_none();
        }
        case StmtKind::TryFallback: {
            const auto* tfs = static_cast<const TryFallbackStmt*>(stmt);
            try {
                for (const auto* s : tfs->try_block->statements) {
                    Value r = execute_stmt(s, env, returned);
                    if (returned) return r;
                }
            } catch (...) {
                for (const auto* s : tfs->fallback_block->statements) {
                    Value r = execute_stmt(s, env, returned);
                    if (returned) return r;
                }
            }
            return Value::make_none();
        }
        case StmtKind::FunctionDecl: {
            const auto* fs = static_cast<const FunctionDeclStmt*>(stmt);
            functions_[fs->name] = fs;
            return Value::make_none();
        }
        default:
            return Value::make_none();
    }
}

Value Runtime::evaluate(const Expr* expr, Environment& env) {
    if (!expr) return Value::make_none();

    switch (expr->kind) {
        case ExprKind::Literal: {
            const auto* lit = static_cast<const LiteralExpr*>(expr);
            switch (lit->type_kind) {
                case TypeKind::Int: return Value::make_int(lit->int_val);
                case TypeKind::Real: return Value::make_real(lit->real_val);
                case TypeKind::Complex: return Value::make_complex(lit->real_val, lit->imag_val);
                case TypeKind::Bool: return Value::make_bool(lit->bool_val);
                case TypeKind::String: return Value::make_string(lit->str_val);
                case TypeKind::None: return Value::make_none();
                default: return Value::make_none();
            }
        }
        case ExprKind::Identifier: {
            const auto* id = static_cast<const IdentExpr*>(expr);
            Value* v = env.get(id->name);
            if (!v) {
                throw std::runtime_error("Variable non définie : " + id->name);
            }
            return *v;
        }
        case ExprKind::Unary: {
            const auto* u = static_cast<const UnaryExpr*>(expr);
            Value opnd = evaluate(u->operand, env);
            if (u->op == TokenKind::NotKw) {
                if (opnd.kind != ValueKind::Bool) throw std::runtime_error("'not' attend un booléen");
                return Value::make_bool(!opnd.bool_val);
            }
            if (u->op == TokenKind::Minus) {
                if (opnd.kind == ValueKind::Int) return Value::make_int(-opnd.int_val);
                if (opnd.kind == ValueKind::Real) return Value::make_real(-opnd.real_val);
                if (opnd.kind == ValueKind::Complex) return Value::make_complex(-opnd.complex_val.real(), -opnd.complex_val.imag());
                if (opnd.kind == ValueKind::Quaternion) return Value::make_quaternion(-opnd.quat_val.r, -opnd.quat_val.i, -opnd.quat_val.j, -opnd.quat_val.k);
            }
            if (u->op == TokenKind::Plus) return opnd;
            throw std::runtime_error("Opérateur unaire non supporté");
        }
        case ExprKind::Binary: {
            const auto* b = static_cast<const BinaryExpr*>(expr);
            // Court-circuit pour and / or
            if (b->op == TokenKind::AndKw) {
                Value l = evaluate(b->left, env);
                if (l.kind != ValueKind::Bool) throw std::runtime_error("'and' attend un booléen");
                if (!l.bool_val) return Value::make_bool(false);
                Value r = evaluate(b->right, env);
                if (r.kind != ValueKind::Bool) throw std::runtime_error("'and' attend un booléen");
                return Value::make_bool(r.bool_val);
            }
            if (b->op == TokenKind::OrKw) {
                Value l = evaluate(b->left, env);
                if (l.kind != ValueKind::Bool) throw std::runtime_error("'or' attend un booléen");
                if (l.bool_val) return Value::make_bool(true);
                Value r = evaluate(b->right, env);
                if (r.kind != ValueKind::Bool) throw std::runtime_error("'or' attend un booléen");
                return Value::make_bool(r.bool_val);
            }

            Value left = evaluate(b->left, env);
            Value right = evaluate(b->right, env);
            return apply_binary_op(b->op, left, right);
        }
        case ExprKind::Call: {
            const auto* c = static_cast<const CallExpr*>(expr);
            // Fonctions built-in
            if (c->callee == "print") {
                std::string out;
                for (size_t i = 0; i < c->args.size(); ++i) {
                    Value v = evaluate(c->args[i], env);
                    // Résolution d'interpolation de chaîne de base
                    std::string s = v.to_string();
                    size_t pos = 0;
                    while ((pos = s.find('{', pos)) != std::string::npos) {
                        size_t end_pos = s.find('}', pos);
                        if (end_pos != std::string::npos) {
                            std::string var_name = s.substr(pos + 1, end_pos - pos - 1);
                            Value* interp_val = env.get(var_name);
                            if (interp_val) {
                                std::string replacement = interp_val->to_string();
                                s.replace(pos, end_pos - pos + 1, replacement);
                                pos += replacement.length();
                            } else {
                                pos = end_pos + 1;
                            }
                        } else {
                            break;
                        }
                    }
                    if (i > 0) out += " ";
                    out += s;
                }
                stdout_log_.push_back(out);
                return Value::make_none();
            }
            if (c->callee == "quaternion") {
                if (c->args.size() != 4) throw std::runtime_error("quaternion(...) attend 4 arguments");
                double r = to_real(evaluate(c->args[0], env));
                double i = to_real(evaluate(c->args[1], env));
                double j = to_real(evaluate(c->args[2], env));
                double k = to_real(evaluate(c->args[3], env));
                return Value::make_quaternion(r, i, j, k);
            }
            if (c->callee == "abs") {
                if (c->args.size() != 1) throw std::runtime_error("abs(...) attend 1 argument");
                Value v = evaluate(c->args[0], env);
                if (v.kind == ValueKind::Int) return Value::make_int(std::abs(v.int_val));
                if (v.kind == ValueKind::Real) return Value::make_real(std::abs(v.real_val));
                if (v.kind == ValueKind::Complex) return Value::make_real(std::abs(v.complex_val));
                if (v.kind == ValueKind::Quaternion) return Value::make_real(v.quat_val.norm());
                throw std::runtime_error("abs(...) non supporté pour ce type");
            }
            if (c->callee == "exp") {
                if (c->args.size() != 1) throw std::runtime_error("exp(...) attend 1 argument");
                Value v = evaluate(c->args[0], env);
                if (v.kind == ValueKind::Real || v.kind == ValueKind::Int) return Value::make_real(std::exp(to_real(v)));
                if (v.kind == ValueKind::Complex) {
                    auto res = std::exp(v.complex_val);
                    return Value::make_complex(res.real(), res.imag());
                }
                throw std::runtime_error("exp(...) pour ce type non implémenté");
            }
            if (c->callee == "ln") {
                if (c->args.size() != 1) throw std::runtime_error("ln(...) attend 1 argument");
                Value v = evaluate(c->args[0], env);
                if (v.kind == ValueKind::Real || v.kind == ValueKind::Int) return Value::make_real(std::log(to_real(v)));
                if (v.kind == ValueKind::Complex) {
                    auto res = std::log(v.complex_val);
                    return Value::make_complex(res.real(), res.imag());
                }
                throw std::runtime_error("ln(...) pour ce type non implémenté");
            }
            if (c->callee == "sum") {
                if (c->args.size() != 1) throw std::runtime_error("sum(...) attend 1 tenseur");
                Value v = evaluate(c->args[0], env);
                if (v.kind != ValueKind::Tensor) throw std::runtime_error("sum(...) attend un tenseur");
                double acc = 0.0;
                for (double d : v.tensor_val.data) acc += d;
                return Value::make_real(acc);
            }

            // Appel de fonction utilisateur nommée
            auto it = functions_.find(c->callee);
            if (it != functions_.end()) {
                const FunctionDeclStmt* fn = it->second;
                if (fn->params.size() != c->args.size()) {
                    throw std::runtime_error("Nombre d'arguments incorrect pour " + c->callee);
                }
                Environment fn_env(&env);
                for (size_t i = 0; i < fn->params.size(); ++i) {
                    fn_env.set(fn->params[i].name, evaluate(c->args[i], env));
                }
                bool ret = false;
                for (const auto* s : fn->body->statements) {
                    Value res = execute_stmt(s, fn_env, ret);
                    if (ret) return res;
                }
                return Value::make_none();
            }

            throw std::runtime_error("Fonction non définie : " + c->callee);
        }
        case ExprKind::TensorIndex: {
            const auto* ti = static_cast<const TensorIndexExpr*>(expr);
            Value t_val = evaluate(ti->tensor, env);
            if (t_val.kind != ValueKind::Tensor) throw std::runtime_error("Indexation sur un type non-tenseur");
            std::vector<int64_t> idxs;
            for (const auto* ie : ti->indices) {
                Value iv = evaluate(ie, env);
                if (iv.kind != ValueKind::Int) throw std::runtime_error("Indices de tenseur doivent être int");
                idxs.push_back(iv.int_val);
            }
            return Value::make_real(t_val.tensor_val.get(idxs));
        }
        case ExprKind::TensorLiteral: {
            const auto* tl = static_cast<const TensorLiteralExpr*>(expr);
            std::vector<double> elements;
            for (const auto* e : tl->elements) {
                elements.push_back(to_real(evaluate(e, env)));
            }
            std::vector<int64_t> s = {static_cast<int64_t>(elements.size())};
            return Value::make_tensor(Tensor(s, std::move(elements)));
        }
        case ExprKind::Copy: {
            const auto* cp = static_cast<const CopyExpr*>(expr);
            Value v = evaluate(cp->operand, env);
            // Retourne une copie profonde
            return v;
        }
        case ExprKind::Lambda: {
            // Dans ce premier palier, évaluation immédiate ou fermeture
            return Value::make_none();
        }
    }
    return Value::make_none();
}

Value Runtime::apply_binary_op(TokenKind op, const Value& left, const Value& right) {
    // Concaténation de chaînes
    if (op == TokenKind::Plus && left.kind == ValueKind::String && right.kind == ValueKind::String) {
        return Value::make_string(left.str_val + right.str_val);
    }

    // Égalité
    if (op == TokenKind::EqualEqual) {
        if (left.kind == ValueKind::None && right.kind == ValueKind::None) return Value::make_bool(true);
        if (left.kind == ValueKind::Bool && right.kind == ValueKind::Bool) return Value::make_bool(left.bool_val == right.bool_val);
        if (left.kind == ValueKind::String && right.kind == ValueKind::String) return Value::make_bool(left.str_val == right.str_val);

        // Égalité numérique
        ValueKind widest = get_widest_numeric_type(left.kind, right.kind);
        if (widest == ValueKind::Int) return Value::make_bool(left.int_val == right.int_val);
        if (widest == ValueKind::Real) return Value::make_bool(to_real(left) == to_real(right));
        if (widest == ValueKind::Complex) return Value::make_bool(to_complex(left) == to_complex(right));
        if (widest == ValueKind::Quaternion) return Value::make_bool(to_quaternion(left) == to_quaternion(right));
    }
    if (op == TokenKind::NotEqual) {
        Value eq = apply_binary_op(TokenKind::EqualEqual, left, right);
        return Value::make_bool(!eq.bool_val);
    }

    // Comparaisons d'ordre : interdites pour complex et quaternion !
    if (op == TokenKind::LessEqual || op == TokenKind::GreaterEqual || op == TokenKind::LAngle || op == TokenKind::RAngle) {
        if (left.kind == ValueKind::Complex || right.kind == ValueKind::Complex ||
            left.kind == ValueKind::Quaternion || right.kind == ValueKind::Quaternion) {
            throw std::runtime_error("Les comparaisons d'ordre sont interdites pour complex et quaternion (pas d'ordre total)");
        }
        double l = to_real(left);
        double r = to_real(right);
        if (op == TokenKind::LAngle) return Value::make_bool(l < r);
        if (op == TokenKind::RAngle) return Value::make_bool(l > r);
        if (op == TokenKind::LessEqual) return Value::make_bool(l <= r);
        if (op == TokenKind::GreaterEqual) return Value::make_bool(l >= r);
    }

    // Arithmétique avec promotion numérique canonique
    ValueKind widest = get_widest_numeric_type(left.kind, right.kind);

    // Cas Int
    if (widest == ValueKind::Int) {
        if (op == TokenKind::Plus) return Value::make_int(left.int_val + right.int_val);
        if (op == TokenKind::Minus) return Value::make_int(left.int_val - right.int_val);
        if (op == TokenKind::Star) return Value::make_int(left.int_val * right.int_val);
        if (op == TokenKind::Slash) {
            // Division de deux entiers produit un 'real' selon la spec Onyx !
            if (right.int_val == 0) throw std::runtime_error("Division par zéro");
            return Value::make_real(static_cast<double>(left.int_val) / static_cast<double>(right.int_val));
        }
        if (op == TokenKind::DoubleSlash) {
            if (right.int_val == 0) throw std::runtime_error("Division entière par zéro");
            return Value::make_int(left.int_val / right.int_val);
        }
        if (op == TokenKind::Percent) {
            if (right.int_val == 0) throw std::runtime_error("Modulo par zéro");
            return Value::make_int(left.int_val % right.int_val);
        }
        if (op == TokenKind::Caret) {
            return Value::make_real(std::pow(static_cast<double>(left.int_val), static_cast<double>(right.int_val)));
        }
    }

    // Cas Real
    if (widest == ValueKind::Real) {
        double l = to_real(left);
        double r = to_real(right);
        if (op == TokenKind::Plus) return Value::make_real(l + r);
        if (op == TokenKind::Minus) return Value::make_real(l - r);
        if (op == TokenKind::Star) return Value::make_real(l * r);
        if (op == TokenKind::Slash) {
            if (r == 0.0) throw std::runtime_error("Division par zéro");
            return Value::make_real(l / r);
        }
        if (op == TokenKind::Caret) return Value::make_real(std::pow(l, r));
    }

    // Cas Complex
    if (widest == ValueKind::Complex) {
        auto l = to_complex(left);
        auto r = to_complex(right);
        if (op == TokenKind::Plus) {
            auto res = l + r;
            return Value::make_complex(res.real(), res.imag());
        }
        if (op == TokenKind::Minus) {
            auto res = l - r;
            return Value::make_complex(res.real(), res.imag());
        }
        if (op == TokenKind::Star) {
            auto res = l * r;
            return Value::make_complex(res.real(), res.imag());
        }
        if (op == TokenKind::Slash) {
            auto res = l / r;
            return Value::make_complex(res.real(), res.imag());
        }
    }

    // Cas Quaternion
    if (widest == ValueKind::Quaternion) {
        Quaternion l = to_quaternion(left);
        Quaternion r = to_quaternion(right);
        if (op == TokenKind::Plus) return Value::make_quaternion((l + r).r, (l + r).i, (l + r).j, (l + r).k);
        if (op == TokenKind::Minus) return Value::make_quaternion((l - r).r, (l - r).i, (l - r).j, (l - r).k);
        if (op == TokenKind::Star) {
            Quaternion res = l * r; // Non commutatif !
            return Value::make_quaternion(res.r, res.i, res.j, res.k);
        }
        if (op == TokenKind::Slash) {
            Quaternion res = l / r;
            return Value::make_quaternion(res.r, res.i, res.j, res.k);
        }
    }

    throw std::runtime_error("Opérateur non supporté pour ces opérandes");
}

} // namespace onyx
