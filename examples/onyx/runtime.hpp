#pragma once

#include "ast.hpp"
#include "value.hpp"
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include <iostream>

namespace onyx {

class Environment {
public:
    explicit Environment(Environment* parent = nullptr) : parent_(parent) {}

    void set(const std::string& name, Value val) {
        values_[name] = std::move(val);
    }

    [[nodiscard]] Value* get(const std::string& name) {
        auto it = values_.find(name);
        if (it != values_.end()) {
            return &it->second;
        }
        if (parent_) {
            return parent_->get(name);
        }
        return nullptr;
    }

    [[nodiscard]] bool contains(const std::string& name) const {
        if (values_.find(name) != values_.end()) return true;
        if (parent_) return parent_->contains(name);
        return false;
    }

private:
    Environment* parent_{nullptr};
    std::unordered_map<std::string, Value> values_;
};

class Runtime {
public:
    Runtime();

    /**
     * @brief Exécute un bloc d'instructions racine.
     */
    Value execute(const BlockStmt* root);

    /**
     * @brief Évalue une expression individuelle.
     */
    Value evaluate(const Expr* expr, Environment& env);

    /**
     * @brief Récupère les sorties imprimées par `print(...)`.
     */
    [[nodiscard]] const std::vector<std::string>& get_output() const noexcept {
        return stdout_log_;
    }

    void clear_output() noexcept {
        stdout_log_.clear();
    }

private:
    Value execute_stmt(const Stmt* stmt, Environment& env, bool& returned);
    Value apply_binary_op(TokenKind op, const Value& left, const Value& right);

    std::vector<std::string> stdout_log_;
    std::unordered_map<std::string, const FunctionDeclStmt*> functions_;
};

} // namespace onyx
