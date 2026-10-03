# Playbook : Implémentation d'un Nouveau Langage depuis une Spécification

Ce playbook définit le protocole industriel strict pour concevoir, implémenter et valider un nouveau langage de programmation (compilateur, parseur, runtime système) à partir d'une spécification formelle (ex: `.harness/knowledge/domain/DOCUMENTATIONS.md`).

---

## 1. Les Quatre Piliers Obligatoires

Toute implémentation de langage menée par un agent d'IA doit reposer sur les quatre piliers suivants sans dérogation possible :

### Pilier 1 : La Porte de Validation Humaine (Human-in-the-Loop Gate)
- **Principe d'immutabilité sémantique :** L'agent d'IA n'est jamais autorisé à modifier unilatéralement la sémantique, la syntaxe ou les invariants de la spécification.
- **Points d'arrêt obligatoires :** Dès qu'un cas limite, une ambiguïté grammaticale ou un conflit d'associativité est détecté, l'agent doit formuler une proposition d'arbitrage claire et **attendre la validation humaine explicite** avant de modifier l'AST ou le moteur.
- **Garantie d'alignement :** Aucune décision de conception majeure n'est prise sans inscription datée dans `.harness/knowledge/decisions.md`.

### Pilier 2 : Le Corpus Gelé (Frozen Reference Corpus)
- **Suite d'or immuable (*Golden Test Suite*) :** Un ensemble de snippets représentatifs issus directement de la spécification est figé dès le départ.
- **Règle de non-régression absolue :** Une fois qu'un snippet du corpus gelé compile et s'exécute correctement, il est interdit de modifier ses assertions de test. Toute modification ultérieure du compilateur doit préserver l'intégralité du corpus au vert.
- **Traçabilité des versions :** Tout changement de la spécification entraîne un nouveau numéro de version du corpus de référence.

### Pilier 3 : Le Fuzzing (Fuzz Testing Syntaxique & Mémoire)
- **Fuzzing Lexer/Parser (*Crash-free guarantee*) :** Le parseur Pratt et le lexer doivent être soumis à des entrées aléatoirement corrompues (indentations négatives, blocs non fermés, opérateurs consécutifs, caractères UTF-8 inattendus). Le compilateur doit retourner des erreurs propres sans jamais déclencher de `segfault`, d'avortement inopiné ou de boucle infinie.
- **Audit Sanitizers (ASan + UBsan) :** Tout programme, qu'il soit syntaxiquement valide ou rejeté avec une erreur, doit s'exécuter avec **0 fuite mémoire** et **0 comportement indéfini**. L'allocateur d'arène doit garantir la libération complète des ressources lors d'une erreur d'analyse.

### Pilier 4 : L'Aller-Retour (Round-Trip Verification)
- **Boucle fermée de cohérence :**
  $$\text{Code Source} \xrightarrow{\text{Parse}} \text{AST} \xrightarrow{\text{Pretty-Print}} \text{Code Source}' \xrightarrow{\text{Re-Parse}} \text{AST}' \quad (\text{AST} \equiv \text{AST}')$$
- **Vérification d'idempotence :** Tout arbre de syntaxe abstrait doit pouvoir être réémis et réanalysé en conservant exactement la même structure et les mêmes types.
- **Confrontation Runtime :** L'évaluation de l'AST doit être systématiquement confrontée aux exemples numériques de la spécification (ex: algèbre des quaternions, promotion $int \to real \to complex \to quaternion$, détection statique du *use-after-consume*).

---

## 2. Procédure d'Implémentation Pas-à-Pas

### Étape 1 : Ingestion et Gel de la Spécification
1. Déposer la spécification formelle dans `.harness/knowledge/domain/`.
2. Extraire la table des opérateurs, priorités de liaison et associativités.
3. Rédiger les snippets du corpus de référence initial dans une suite de tests unitaires.

### Étape 2 : Infrastructure Mémoire Déterministe
1. Concevoir l'allocateur d'arène contigu (`Arena`) sans Garbage Collector.
2. Implémenter le suivi déterministe des destructeurs non-triviaux pour garantir 0 fuite sous AddressSanitizer.

### Étape 3 : Lexer Indenté & Parseur Pratt
1. Implémenter la pile d'indentation (`INDENT` / `DEDENT` / `NEWLINE`).
2. Écrire le parseur d'expressions selon l'algorithme de Pratt (Niveaux de priorité stricts, associativité à droite pour la puissance `^`).
3. Écrire le parseur d'instructions (blocs `: `, déclarations, assignations, retours `=>`, `try/fallback`).

### Étape 4 : Vérification Statique (Typage Linéaire & Règle d'Or)
1. Construire l'analyseur de durée de vie et de consommation affine :
   - Variable linéaire consommée lors d'un passage à droite d'une assignation.
   - Protection de persistance via le modificateur `!i`.
   - Duplication explicite via `copy()`.
2. Détecter et rejeter à la compilation toute réutilisation après consommation (*use-after-consume*).

### Étape 5 : Banc de Fuzzing et Juge Souverain
1. Exécuter la suite de fuzzing syntaxique pour valider la robustesse aux entrées malveillantes.
2. Valider l'intégralité sous `-fsanitize=address,undefined -Wall -Wextra -Wconversion -Werror`.
3. Intégrer la cible dans `native/Makefile` et `scripts/verify.sh`.
4. Présenter le rapport de conformité pour franchir la **Porte de Validation Humaine**.
