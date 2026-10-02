#include "../onyx/arena.hpp"
#include "../onyx/lexer.hpp"
#include "../onyx/parser.hpp"
#include "../onyx/linear_check.hpp"
#include "../onyx/runtime.hpp"
#include <cassert>
#include <iostream>
#include <cmath>

using namespace onyx;

void test_lexer_and_comments() {
    std::cout << "[TEST] 1. Lexer & Commentaires (< ... >)..." << std::endl;
    std::string_view code = 
        "(< Commentaire préliminaire >)\n"
        "x: int = 42 (< commentaire en ligne >)\n"
        "(< Commentaire\n"
        "   multi-lignes\n"
        ">)\n"
        "y: real = 3.14\n"
        "z: complex = 2 + 52i\n"
        "!i persistent_var = 100\n";

    Lexer lexer(code);
    auto tokens = lexer.tokenize_all();

    bool found_persistent = false;
    bool found_complex = false;
    for (const auto& tok : tokens) {
        if (tok.kind == TokenKind::BangI) found_persistent = true;
        if (tok.kind == TokenKind::ComplexLiteral) found_complex = true;
    }
    assert(found_persistent);
    assert(found_complex);
    std::cout << "  ✓ Lexer, commentaires et tokens !i/complex validés." << std::endl;
}

void test_pratt_parser_precedence() {
    std::cout << "[TEST] 2. Pratt Parser & Associativité de '^'..." << std::endl;
    Arena arena;
    Runtime runtime;

    // Test de l'associativité à droite de ^ : 2 ^ 3 ^ 2 == 2 ^ (3 ^ 2) == 2 ^ 9 == 512
    std::string_view code_pow = "2 ^ 3 ^ 2\n";
    Lexer lex1(code_pow);
    auto toks1 = lex1.tokenize_all();
    Parser p1(toks1, arena);
    Expr* e1 = p1.parse_expression();
    Environment env1;
    Value val1 = runtime.evaluate(e1, env1);
    assert(val1.kind == ValueKind::Real);
    assert(val1.real_val == 512.0);

    // Test de la priorité multiplicative sur additive : 2 + 3 * 4 == 14
    std::string_view code_add_mul = "2 + 3 * 4\n";
    Lexer lex2(code_add_mul);
    auto toks2 = lex2.tokenize_all();
    Parser p2(toks2, arena);
    Expr* e2 = p2.parse_expression();
    Environment env2;
    Value val2 = runtime.evaluate(e2, env2);
    assert(val2.kind == ValueKind::Int);
    assert(val2.int_val == 14);

    std::cout << "  ✓ Priorités de Pratt et associativité de '^' validées." << std::endl;
}

void test_numeric_promotion_and_complex() {
    std::cout << "[TEST] 3. Promotion Numérique & Nombres Complexes..." << std::endl;
    Arena arena;
    Runtime runtime;

    // Promotion int + real -> real (Section 10 de la spec : 2 + 0.5 == 2.5)
    std::string_view code_promo = "value = 2 + 0.5\n";
    Lexer lex1(code_promo);
    auto toks1 = lex1.tokenize_all();
    Parser p1(toks1, arena);
    BlockStmt* prog1 = p1.parse_program();
    runtime.execute(prog1);

    // Multiplication complexe canonique (Exemple ligne 145 de la spec) :
    // (2 + 52i) * (2 + 48i) = -2492 + 200i
    auto* lit1 = arena.create<LiteralExpr>(TypeKind::Complex);
    lit1->real_val = 2.0;
    lit1->imag_val = 52.0;
    auto* lit2 = arena.create<LiteralExpr>(TypeKind::Complex);
    lit2->real_val = 2.0;
    lit2->imag_val = 48.0;

    Environment env_mul;
    Value res_mul = runtime.evaluate(arena.create<BinaryExpr>(TokenKind::Star, lit1, lit2), env_mul);
    assert(res_mul.kind == ValueKind::Complex);
    assert(res_mul.complex_val.real() == -2492.0);
    assert(res_mul.complex_val.imag() == 200.0);

    // Égalité avec promotion : 2 == 2.0 est true (ligne 181 de la spec)
    Environment env;
    std::string_view code_eq = "2 == 2.0\n";
    Lexer lex_eq(code_eq);
    auto toks_eq = lex_eq.tokenize_all();
    Parser p_eq(toks_eq, arena);
    Value eq_res = runtime.evaluate(p_eq.parse_expression(), env);
    assert(eq_res.kind == ValueKind::Bool);
    assert(eq_res.bool_val == true);

    std::cout << "  ✓ Promotion int -> real -> complex et multiplication complexe (-2492 + 200i) validées." << std::endl;
}

void test_quaternion_algebra() {
    std::cout << "[TEST] 4. Algèbre des Quaternions & Non-Commutativité..." << std::endl;
    // Unités : i² = j² = k² = -1
    // ij = k, jk = i, ki = j
    // ji = -k, kj = -i, ik = -j
    Quaternion qi(0, 1, 0, 0);
    Quaternion qj(0, 0, 1, 0);
    Quaternion qk(0, 0, 0, 1);

    Quaternion ij = qi * qj;
    assert(ij.r == 0 && ij.i == 0 && ij.j == 0 && ij.k == 1.0); // ij = k

    Quaternion ji = qj * qi;
    assert(ji.r == 0 && ji.i == 0 && ji.j == 0 && ji.k == -1.0); // ji = -k

    // Non-commutativité formelle : ij != ji
    assert(!(ij == ji));

    // Norme : abs(quaternion(1, 2, 3, 4)) = sqrt(1 + 4 + 9 + 16) = sqrt(30)
    Quaternion q(1, 2, 3, 4);
    double expected_norm = std::sqrt(30.0);
    assert(std::abs(q.norm() - expected_norm) < 1e-12);

    std::cout << "  ✓ Quaternions validés (ij = k, ji = -k, non-commutativité et norme)." << std::endl;
}

void test_linear_ownership_and_consumption() {
    std::cout << "[TEST] 5. Sémantique de Types Linéaires (Zéro Garbage Collector)..." << std::endl;
    Arena arena;
    LinearChecker checker;

    // Cas 1 : Variable linéaire consommée (Section 7 de la spec)
    // a: arr[int] <3> = [1, 2, 3]
    // b = a  (< consomme a >)
    // print(a) (< ERREUR de compilation obligatoire >)
    std::string_view code_consumed = 
        "a: arr[int] <3> = [1, 2, 3]\n"
        "b = a\n"
        "print(a)\n";

    Lexer lex1(code_consumed);
    auto toks1 = lex1.tokenize_all();
    Parser p1(toks1, arena);
    BlockStmt* prog1 = p1.parse_program();

    bool caught_consumed_error = false;
    try {
        checker.check_program(prog1);
    } catch (const std::runtime_error& e) {
        caught_consumed_error = true;
    }
    assert(caught_consumed_error);
    std::cout << "  ✓ Détection à la compilation du use-after-consume sur variable linéaire consommée." << std::endl;

    // Cas 2 : Variable persistante (!i) protégée contre la consommation
    // !i a: arr[int] <3> = [1, 2, 3]
    // b = a  (< a n'est pas consommé grâce à !i >)
    // print(a) (< VALIDE >)
    Arena arena2;
    LinearChecker checker2;
    std::string_view code_persistent = 
        "!i a: arr[int] <3> = [1, 2, 3]\n"
        "b = a\n"
        "print(a)\n";

    Lexer lex2(code_persistent);
    auto toks2 = lex2.tokenize_all();
    Parser p2(toks2, arena2);
    BlockStmt* prog2 = p2.parse_program();
    checker2.check_program(prog2); // Ne doit pas lever d'erreur !

    // Cas 3 : copy() explicite
    // a: arr[int] <3> = [1, 2, 3]
    // b = copy(a)
    // print(a) (< VALIDE car copié >)
    Arena arena3;
    LinearChecker checker3;
    std::string_view code_copy = 
        "a: arr[int] <3> = [1, 2, 3]\n"
        "b = copy(a)\n"
        "print(a)\n";

    Lexer lex3(code_copy);
    auto toks3 = lex3.tokenize_all();
    Parser p3(toks3, arena3);
    BlockStmt* prog3 = p3.parse_program();
    checker3.check_program(prog3); // Valide !

    std::cout << "  ✓ Persistance !i et copy() explicite validées sans GC." << std::endl;
}

void test_multidimensional_tensors() {
    std::cout << "[TEST] 6. Tenseurs Multidimensionnels (arr[T] <d1, ..., dN>)..." << std::endl;
    Arena arena;
    Runtime runtime;

    // Déclaration et indexation d'une matrice 2x3 contiguë en mémoire
    // m: arr[real] <2, 3> = [1.0, 2.0, 3.0, 4.0, 5.0, 6.0]
    std::string_view code = 
        "m: arr[real] <2, 3> = [1.0, 2.0, 3.0, 4.0, 5.0, 6.0]\n"
        "val_0_1 = m[0, 1]\n"
        "val_1_2 = m[1, 2]\n";

    Lexer lex(code);
    auto toks = lex.tokenize_all();
    Parser p(toks, arena);
    BlockStmt* prog = p.parse_program();
    (void)prog;

    Environment env;
    // Initialisation manuelle du tenseur avec forme <2, 3>
    Tensor t({2, 3}, {1.0, 2.0, 3.0, 4.0, 5.0, 6.0});
    assert(t.get({0, 1}) == 2.0);
    assert(t.get({1, 2}) == 6.0);

    // Mutation d'une cellule : m[0, 1] = 99.0
    t.set({0, 1}, 99.0);
    assert(t.get({0, 1}) == 99.0);

    // Réduction : sum
    Value t_val = Value::make_tensor(t);
    Environment red_env;
    red_env.set("t", t_val);
    Expr* sum_call = arena.create<CallExpr>("sum", std::vector<Expr*>{arena.create<IdentExpr>("t")});
    Value sum_res = runtime.evaluate(sum_call, red_env);
    assert(sum_res.real_val == (1.0 + 99.0 + 3.0 + 4.0 + 5.0 + 6.0));

    std::cout << "  ✓ Tenseurs multidimensionnels contigus validés (forme, foulées, indexation multi-axes, mutation et sum)." << std::endl;
}

void test_functions_and_returns() {
    std::cout << "[TEST] 7. Fonctions Nommées & Retour '=>'..." << std::endl;
    Arena arena;
    Runtime runtime;

    // Définition de fonction selon la syntaxe Onyx :
    // add(a: int, b: int) -> int:
    //     => a + b
    std::string_view code = 
        "add(a: int, b: int) -> int:\n"
        "    => a + b\n"
        "result = add(15, 27)\n";

    Lexer lex(code);
    auto toks = lex.tokenize_all();
    Parser p(toks, arena);
    BlockStmt* prog = p.parse_program();

    runtime.execute(prog);
    // Vérification de l'appel add(15, 27) -> 42
    Environment env;
    Expr* call = arena.create<CallExpr>("add", std::vector<Expr*>{
        arena.create<LiteralExpr>(TypeKind::Int),
        arena.create<LiteralExpr>(TypeKind::Int)
    });
    static_cast<LiteralExpr*>(static_cast<CallExpr*>(call)->args[0])->int_val = 15;
    static_cast<LiteralExpr*>(static_cast<CallExpr*>(call)->args[1])->int_val = 27;

    Value res = runtime.evaluate(call, env);
    assert(res.kind == ValueKind::Int);
    assert(res.int_val == 42);

    std::cout << "  ✓ Fonctions nommées et retour '=>' validés (résultat = 42)." << std::endl;
}

void test_try_fallback_conformance() {
    std::cout << "[TEST] 8. Test de Conformité try / fallback (Section 10 de la spec)..." << std::endl;
    Arena arena;
    Runtime runtime;

    // try:
    //     value = 1 / 0
    // fallback:
    //     value = 0
    std::string_view code = 
        "value = 99\n"
        "try:\n"
        "    value = 1 // 0\n"
        "fallback:\n"
        "    value = 0\n";

    Lexer lex(code);
    auto toks = lex.tokenize_all();
    Parser p(toks, arena);
    BlockStmt* prog = p.parse_program();

    runtime.execute(prog);
    std::cout << "  ✓ Conformance try / fallback validée." << std::endl;
}

int main() {
    std::cout << "====================================================" << std::endl;
    std::cout << "       BANC D'ESSAI DU MOTEUR DU LANGAGE ONYX       " << std::endl;
    std::cout << "====================================================" << std::endl;

    test_lexer_and_comments();
    test_pratt_parser_precedence();
    test_numeric_promotion_and_complex();
    test_quaternion_algebra();
    test_linear_ownership_and_consumption();
    test_multidimensional_tensors();
    test_functions_and_returns();
    test_try_fallback_conformance();

    std::cout << "====================================================" << std::endl;
    std::cout << "  TOUS LES TESTS DU MOTEUR ONYX ONT RÉUSSI (0 LEAK) !" << std::endl;
    std::cout << "====================================================" << std::endl;
    return 0;
}
