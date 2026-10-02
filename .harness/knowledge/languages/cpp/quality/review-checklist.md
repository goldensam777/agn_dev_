# Checklist de Review & Outillage Qualité

> **Usage** : le reviewer charge ce fichier et applique chaque section. Une PR "de qualité"
> = toutes les cases cochées ou l'exception documentée. Les outils listés en section 4 sont
> le filet de sécurité : **ils complètent la review, ils ne la remplacent pas** (les outils
> statiques ne voient pas ~la moitié des vraies violations runtime — étude VulBench-CPP :
> accord quasi nul entre analyse statique et dynamique ; le code généré par IA déclenche
> ~2× plus de violations runtime que le code humain : la barre doit donc être *plus haute*).

---

## 1. Corrections (le code marche-t-il vraiment ?)

- [ ] Tests présents pour le nouveau comportement ET le chemin d'erreur
- [ ] Le test prouve quelque chose (référence indépendante, pas juste "ça passe")
- [ ] Pas de `TODO`/`FIXME`/`XXX` introduit sans ticket
- [ ] Pas de code mort, de paramètre ignoré, de branche dupliquée
- [ ] Pas de warning nouveau du compilateur (`-Wall -Wextra -Werror` propres)

## 2. Robustesse (que se passe-t-il dans les mauvais cas ?)

- [ ] Toute entrée externe validée (tailles, bornes, encodage) à la frontière
- [ ] Garantie d'exception-safety raisonnée (nothrow/forte/base) et tenue via RAII
- [ ] Aucun `catch` qui avale ; chaque erreur traitée à un niveau qui peut agir
- [ ] Aucun UB plausible : overflow signé, dangling, use-after-move, type-punning
- [ ] Arithmétique d'indices/itérateurs : bornes prouvées ou `.at()`/spans
- [ ] Concurrence : données partagées sous synchronisation, captures async par valeur

## 3. Qualité de conception (est-ce le bon code ?)

- [ ] Ownership visible dans les types (cf. ownership-raii.md) ; Rule of Zero
- [ ] `const`-correctness complet ; aucun `const_cast` d'écriture
- [ ] Une fonction = une opération ; sorties retournées ; noms qui disent le quoi
- [ ] États invalides inexpressibles (types forts, enum class, optional/variant)
- [ ] Pas de duplication : le code répété est un candidat extraction
- [ ] La structure des données sert le parcours dominant (cf. memory.md §2)
- [ ] Pas d'optimisation non mesurée ; benchmark ajouté si optimisation
- [ ] Domaine respecté : règles de `domains/*.md` appliquées (numérique, concurrence…)
- [ ] Le diff est minimal : rien de réécrit "au passage" sans lien avec la tâche

## 4. Les trois questions magiques (reprise anti-patterns.md)

1. Qui possède cette ressource, et comment le lecteur le sait-il ?
2. Que se passe-t-il si cette ligne throw ?
3. Cet état peut-il être rendu impossible par le type system ?

---

## 5. Outillage : ce que `verify.sh` / la CI DOIT exécuter

Couche par couche — chacune attrape des classes d'erreurs que les autres ratent :

### 5.1 Compilation stricte (chaque build)
```
-Wall -Wextra -Wpedantic -Werror -Wshadow -Wconversion (ou -Wsign-conversion)
-Wold-style-cast (C++), -Woverloaded-virtual, -Wnull-dereference
```
Warnings = erreurs. Un warning nouveau est un échec de review automatisé.

### 5.2 Analyse statique (chaque PR)
- **clang-tidy** avec `.clang-tidy` versionné (checks à fort signal :
  `bugprone-*`, `cppcoreguidelines-*` ciblés, `performance-*`, `modernize-*`,
  `readability-*` sélectif, `clang-analyzer-*`). Fixes automatiques appliquées quand sûres.
- **cppcheck** en complément (bon sur les fuites C-style et les bounds).

### 5.3 Sanitizers (les tests exécutés 3× en CI, builds séparés)
| Sanitizer | Détecte | Quand |
|---|---|---|
| **ASan** (+ LSan) | out-of-bounds, use-after-free, fuites | build test par défaut |
| **UBSan** | overflow signé, déréférences nulles, shifts, aliasing | build test |
| **TSan** | data races | build test des suites concourantes |
| MSan (Linux/clang) | lectures non initialisées | optionnel, le plus cher |

### 5.4 Tests & couverture
- GoogleTest ou Catch2 ; ctest en CI.
- Couverture de ligne sur le code nouveau (seuil projet, p. ex. 80% sur les modules
  critiques) — avec l'esprit : la couverture mesure les trous, pas la qualité.
- Benchmarks avec seuil de régression (Google Benchmark, médiane) pour le code hot.

### 5.5 Fuzzing (frontières de parsing)
- libFuzzer sur lexer/parsers/désérialiseurs, corpus minimisé commité, exécuté en CI
  (nightly si trop lent) sous ASan/UBSan.

### 5.6 Format & hygiène
- `clang-format` versionné (.clang-format), appliqué en CI (le formatage n'est jamais
  un sujet de review humaine).
- CMake avec `CMAKE_CXX_STANDARD` fixé, pas de glob de sources.

### 5.7 Le multi-tier est non négociable
Résultat mesuré (VulBench-CPP, 8 918 programmes) : les vraies violations runtime
passent à travers l'analyse statique seule ; ~1,8% des programmes qui passent *tous* les
tests déclenchent quand même des sanitizers. Donc : **tests + statique + sanitizers + fuzz**
— enlever une couche, c'est accepter une classe entière de bugs en production.

---

## 6. Scoring proposé pour le reviewer-agent (à brancher sur votre juge)

Chaque défaut trouvé en review est classé :

| Classe | Exemples | Conséquence |
|---|---|---|
| **Bloquant** | UB, fuite, race, violation d'ownership, résultat numériquement faux | rejet, tâche de correction auto |
| **Majeur** | garantie d'exception non tenue, API mal typée, anti-pattern #1–#10 | rejet sauf exception documentée |
| **Mineur** | const manquant, nommage, duplication localisée | commentaire, correction simple |
| **Nit** | style, formatage | ignoré (clang-format s'en charge) |

La sortie du reviewer doit être : liste classée + fichier/ligne + pourquoi + référence au
document de connaissance correspondant (`core/…`, `domains/…`) — pour que chaque review
renforce aussi le corpus.
