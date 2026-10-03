# Moteur de Référence du Langage Onyx

Ce répertoire contient l'implémentation de référence en C++20 du compilateur et runtime du langage système **Onyx**, conçu selon les principes de la Forge Agentique et validé sous AddressSanitizer.

Spécification formelle de référence : [`.harness/knowledge/domain/DOCUMENTATIONS.md`](../../.harness/knowledge/domain/DOCUMENTATIONS.md).

---

## 1. Choix d'Architecture Spécifiques à Onyx

### A. Pile d'Indentation et Commentaires Délimités
- **Lexer (`lexer.hpp` / `lexer.cpp`) :**
  - Syntaxe par indentation sans accolades : émission automatique des tokens virtuels `INDENT`, `DEDENT` et `NEWLINE`.
  - Gestion des délimiteurs de commentaires imbriqués et multilignes `(< ... >)` sans collision avec les opérateurs relationnels `<` et `>`.

### B. Gestion Mémoire par Arène RAII sans Garbage Collector
- **Arène contiguë (`arena.hpp`) :**
  - Allocateur de blocs mémoire contigus de 64 Ko, réduisant drastiquement la fragmentation et maximisant la localité spatiale du cache L1/L2.
  - Suivi déterministe d'une chaîne de destructeurs non-triviaux : garantit **0 fuite mémoire** sous AddressSanitizer même en cas d'interception d'exception ou de rejet syntaxique.

### C. Parseur d'Expressions de Pratt à 10 Niveaux
- **Parseur (`parser.hpp` / `parser.cpp`) :**
  - Implémentation du Pratt Parsing gérant 10 niveaux de priorité de liaison (*binding powers*).
  - Associativité à droite stricte de l'opérateur puissance `^` ($2^{3^2} = 512$).
  - Priorité multiplicative (`*`, `/`, `//`, `%`) sur l'addition/soustraction (`+`, `-`).

### D. Analyse Statique de Typage Linéaire Zéro-GC
- **Vérificateur sémantique (`linear_check.hpp` / `linear_check.cpp`) :**
  - Règle de consommation affine : toute variable linéaire transmise à droite d'une assignation ou passée en argument est consommée à la compilation.
  - Rejet systématique du *use-after-consume* dès la phase d'analyse statique.
  - Modificateur d'immutabilité persistante `!i` pour neutraliser la consommation linéaire.
  - Fonction intrinsèque `copy(val)` pour la duplication explicite et délibérée.

### E. Tour d'Élargissement Numérique et Algèbre
- **Runtime (`runtime.hpp` / `runtime.cpp`, `value.hpp`) :**
  - Hiérarchie de promotion automatique sans perte :
    $$\text{int} \xrightarrow{} \text{real} \xrightarrow{} \text{complex} \xrightarrow{} \text{quaternion}$$
  - Support natif des quaternions non-commutatifs $q = a + bi + cj + dk$.
  - Tenseurs multidimensionnels contigus en mémoire avec indexation multi-axes `tensor[i, j]`.

---

## 2. Validation et Bancs d'Essai

### Suite d'Or Immuable
Fichier de test unitaire : [`examples/test_onyx.cpp`](../test_onyx.cpp).
```bash
make -f native/Makefile test_asan
```

### Campagne de Fuzzing Modulaire
Fichier fuzzer : [`examples/onyx/fuzz_onyx.cpp`](./fuzz_onyx.cpp), branché sur le moteur générique [`native/include/forge/fuzz_engine.hpp`](../../native/include/forge/fuzz_engine.hpp).
- Fuzzing de robustesse (seuil minimal 5000 itérations sous ASan/UBsan) :
  ```bash
  make -f native/Makefile fuzz
  ```
- Fuzzing guidé par la couverture de code (LLVM libFuzzer) :
  ```bash
  make -f native/Makefile fuzz_coverage
  ```
