# Glossaire du Projet & Définitions Clés

Ce glossaire définit les termes récurrents utilisés par l'équipe et les agents dans ce dépôt.

---

### Termes d'Ingénierie Système & Compilateurs
- **ASan (AddressSanitizer) :** Outil de détection rapide des erreurs de mémoire en C/C++ (dépassements de tampon, utilisation après libération `use-after-free`, etc.).
- **UBSan (UndefinedBehaviorSanitizer) :** Détecteur de comportements indéterminés (débordements d'entiers signés, déréférencement de pointeurs nuls, etc.).
- **LLVM :** Infrastructure de compilation fournissant un langage intermédiaire (LLVM IR) et des générateurs de code optimisé pour les processeurs modernes (x86_64, ARM, etc.).
- **AST (Abstract Syntax Tree) :** Arbre syntaxique abstrait représentant la structure d'un code source lors de son analyse par un compilateur.
- **Onyx :** Projet de langage de programmation haute performance conçu au sein de cette forge.
- **Mercuria :** Système et banc de mesure haute précision (harness de micro-benchmarks et profilage continu).

### Termes d'Architecture Logicielle & Fullstack
- **Contracts :** Ensemble de schémas Zod et de spécifications qui scellent le format des données entre le client web et le serveur.
- **Bridge :** Couche logicielle (dans `server/src/bridge/`) qui encapsule les appels système ou liaisons FFI avec le binaire natif (`native/`).
- **Harness :** Environnement d'accompagnement de l'agent (documentation, playbooks, exemples) lui permettant de raisonner sans hallucination.
- **Verify :** Le script d'intégration et de vérification unique (`scripts/verify.sh`) qui tranche le statut d'une modification (PASS ou FAIL).
