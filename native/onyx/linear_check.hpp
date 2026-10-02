#pragma once

#include "ast.hpp"
#include <string>
#include <unordered_map>
#include <vector>
#include <stdexcept>

namespace onyx {

enum class LinearState {
    Valid,
    Consumed
};

struct VarInfo {
    bool is_persistent{false}; // Déclaré avec !i
    bool is_linear{false};     // Tenseur / Array ou ressource linéaire
    LinearState state{LinearState::Valid};
};

class LinearChecker {
public:
    LinearChecker();

    /**
     * @brief Analyse l'AST complet et valide qu'aucune variable consommée n'est réutilisée.
     */
    void check_program(const BlockStmt* root);

private:
    void enter_scope();
    void exit_scope();
    void declare_variable(const std::string& name, bool is_persistent, bool is_linear);
    VarInfo* lookup_variable(const std::string& name);

    void check_stmt(const Stmt* stmt);
    void check_expr(const Expr* expr, bool is_consuming_context = false);

    std::vector<std::unordered_map<std::string, VarInfo>> scopes_;
};

} // namespace onyx
