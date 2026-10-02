# Corpus de Théorie des Compilateurs (Le « Corpus Dragon »)

> Base de connaissances algorithmiques fondamentales pour la conception et l'implémentation de langages, compilateurs, interpréteurs et runtimes. Distillée à partir de l'ouvrage classique *Compilers: Principles, Techniques, & Tools* (Aho, Lam, Sethi, Ullman - 2e édition) et *Crafting Interpreters* (Nystrom).

---

## 1. Organisation du Corpus

| Fichier | Sujet Traité | Référence Dragon Book |
|---|---|---|
| [`01-automates-lexicaux.md`](01-automates-lexicaux.md) | Regex, Thompson (NFA), Déterminisation (DFA), Minimisation de Hopcroft, Buffering | Chapitre 3 (p. 109–189) |
| [`02-analyse-syntaxique-grammaires.md`](02-analyse-syntaxique-grammaires.md) | Grammaires BNF, Ensembles FIRST/FOLLOW, LL(1), Pratt Parser, LR(0), SLR, LR(1), LALR | Chapitre 4 (p. 191–302) |
| [`03-traduction-dirigee-syntaxe-ast.md`](03-traduction-dirigee-syntaxe-ast.md) | Définitions SDD (S/L-attribuées), Schémas SDT, Construction de DAG et d'AST | Chapitre 5 (p. 303–356) |
| [`04-representations-intermediaires-tac-ssa.md`](04-representations-intermediaires-tac-ssa.md) | Code à Trois Adresses (TAC), Forme SSA, Frontières de Dominance, Placement des fonctions $\phi$ | Chapitre 6 (p. 357–426) |
| [`05-environnement-execution-memoire.md`](05-environnement-execution-memoire.md) | Stack frames, Enregistrements d'activation, Chaînes statiques (Closures), Garbage Collection | Chapitre 7 (p. 427–504) |
| [`06-generation-code-blocs-base.md`](06-generation-code-blocs-base.md) | Partitionnement en Blocs de Base, Graphe de Flot (CFG), Sélection d'Instructions (DP/Maxmunch) | Chapitre 8 (p. 505–584) |
| [`07-optimisations-flot-donnees.md`](07-optimisations-flot-donnees.md) | Équations de point fixe, Reaching Defs, Live Variables, Boucles Naturelles, LICM | Chapitre 9 (p. 585–702) |
| [`08-allocation-registres.md`](08-allocation-registres.md) | Graphe d'interférence, Coloration de Chaitin-Briggs (Simplify/Spill/Select), Linear Scan | Chapitre 8 (§8.8, p. 556–567) |

---

## 2. Règles de Chargement pour l'Agent

- Lors de la conception de la **grammaire ou du lexer** : charger `01-automates-lexicaux.md` et `02-analyse-syntaxique-grammaires.md`.
- Lors de la construction de **l'AST ou de l'analyse sémantique** : charger `02-analyse-syntaxique-grammaires.md` et `03-traduction-dirigee-syntaxe-ast.md`.
- Lors de l'émission de **LLVM IR ou d'un bytecode intermédiaire** : charger `04-representations-intermediaires-tac-ssa.md` et `06-generation-code-blocs-base.md`.
- Lors de l'écriture d'un **runtime ou garbage collector** : charger `05-environnement-execution-memoire.md`.
- Lors de l'écriture de **passes d'optimisation ou backends natifs** : charger `07-optimisations-flot-donnees.md` et `08-allocation-registres.md`.

---

## 3. Accès aux Ouvrages Originaux

Pour une citation textuelle ou la vérification d'un théorème spécifique dans l'ouvrage PDF complet sans surcharger le contexte LLM :
```bash
python3 scripts/query_book.py --book dragon --search "Dominance Frontiers"
python3 scripts/query_book.py --book dragon --page 582 --count 2
```
