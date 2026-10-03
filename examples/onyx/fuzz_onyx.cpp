#include "forge/fuzz_engine.hpp"
#include "onyx/arena.hpp"
#include "onyx/lexer.hpp"
#include "onyx/parser.hpp"
#include <iostream>
#include <string>
#include <vector>

namespace onyx::fuzz {

/**
 * @brief Cible de Fuzzing pour le parseur et lexer Onyx
 */
struct OnyxTarget {
    void operator()(std::string_view input) const {
        Arena arena;
        Lexer lexer(input);
        auto tokens = lexer.tokenize_all();

        Parser parser(tokens, arena);
        parser.parse_program();
    }
};

/**
 * @brief Configuration du dictionnaire lexical et des motifs adversariaux d'Onyx
 */
inline void configure_onyx_fuzzer(forge::FuzzEngine& engine) {
    engine.set_tokens({
        "x", "y", "z", "result", "alpha", "temp", "foo", "bar",
        "int", "real", "complex", "quaternion", "bool", "string",
        "if", "elif", "else", "while", "match", "case", "default", "try", "fallback",
        "lambda", "print", "none", "true", "false", ":break", "!i", "copy",
        "0", "1", "42", "3.14159", "2+52i", "0.0", "9999999999",
        "+", "-", "*", "/", "//", "%", "^", "==", "!=", "<", "<=", ">", ">=",
        "=", "+=", "-=", "->", "=>", ":", ",", ".", "[", "]", "(", ")",
        "\"", "'", "(<", ">)", "\n", "    ", "        ", "\t"
    });

    engine.set_adversarial_patterns({
        "if true:\n    x = 1\n  y = 2\n      z = 3\n",
        "(< racine (< imbriqué >) x = 42\n (< non fermé",
        "x = (((2 + * 3 ^ ^ 4 - / 5))))",
        "!i\n!i x: int =\nlambda (a, b) =>\n",
        "t = [1, 2, , 3, [4, 5], ]\nz = 2 + 52i * 3i\n",
        ":break 100\nfallback:\n    print(1)\n"
    });
}

} // namespace onyx::fuzz

#ifndef FORGE_BUILD_LIBFUZZER

int main(int argc, char* argv[]) {
    size_t iterations = 5000;
    uint32_t seed = 42;

    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--iterations") == 0 && i + 1 < argc) {
            iterations = static_cast<size_t>(std::stoul(argv[++i]));
        } else if (std::strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
            seed = static_cast<uint32_t>(std::stoul(argv[++i]));
        }
    }

    std::cout << "==========================================================" << std::endl;
    std::cout << "   FORGE FUZZER : Lexer / Parser Onyx sous ASan & UBsan   " << std::endl;
    std::cout << "==========================================================" << std::endl;

    forge::FuzzEngine engine(seed);
    onyx::fuzz::configure_onyx_fuzzer(engine);

    onyx::fuzz::OnyxTarget target;
    auto report = engine.run(target, iterations, "Onyx Parser");

    return report.passed ? 0 : 1;
}

#else

// Point d'entrée pour le fuzzing guidé par la couverture avec LLVM libFuzzer
FORGE_DEFINE_LIBFUZZER_TARGET(onyx::fuzz::OnyxTarget)

#endif
