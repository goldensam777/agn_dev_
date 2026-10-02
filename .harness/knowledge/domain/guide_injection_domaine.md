# Guide d'Ingestion : Comment Configurer un Nouveau Projet de Laboratoire

Ce document explique aux équipes et aux agents comment configurer ce dépôt pour un nouveau projet d'ingénierie (Langage de programmation, Calcul scientifique, etc.).

---

## 1. Cas d'Usage A : Projet de Langage de Programmation

Pour démarrer la conception d'un nouveau langage (compilateur, interpréteur, DSL) :

1. **Déposer les spécifications syntaxiques dans ce dossier (`.harness/knowledge/domain/`) :**
   - `grammar.ebnf` : Règles de grammaire formelle du langage.
   - `typesystem.md` : Règles de typage (inférence, types dépendants, gestion mémoire).
2. **Définir les étapes dans les playbooks :**
   - `.harness/playbooks/add-ast-node.md` : Guide pour ajouter une expression ou une instruction.
   - `.harness/playbooks/add-llvm-pass.md` : Procédure pour émettre du LLVM IR.
3. **Configurer les tests et bancs de mesure dans `native/` :**
   - Écrire les benchmarks comparatifs (vitesse de parsing, débit de compilation, performance du binaire produit face à C++/Rust).

---

## 2. Cas d'Usage B : Projet Fullstack Scientifique

Pour développer une application de simulation ou d'analyse numérique :

1. **Déposer les modèles mathématiques dans ce dossier (`.harness/knowledge/domain/`) :**
   - `equations.md` : Formules physiques, approximations numériques, contraintes de convergence.
2. **Déclarer les schémas dans `contracts/schemas.ts` :**
   - Schémas Zod des tenseurs, grilles de discrétisation et paramètres de simulation.
3. **Implémenter le moteur dans `native/` :**
   - Algorithmes SIMD / multi-threadés (OpenMP, C++ threads, Rust Rayon).
   - Valider la gestion mémoire sous AddressSanitizer via `bash scripts/verify.sh`.
4. **Visualiser dans `web/` :**
   - Canvases WebGL / WebGPU connectés au serveur via le pont typé.

---

## Règle Absolue pour l'Agent

Avant d'écrire la moindre ligne de code pour un projet, l'agent doit impérativement inspecter le contenu présent dans `.harness/knowledge/domain/` pour s'aligner sur la théorie et les contraintes métier du laboratoire.
