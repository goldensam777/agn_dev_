# Playbook : Initialisation et Maintien d'un Corpus de Connaissances Langage

Ce playbook décrit la procédure standardisée permettant à un agent d'IA d'auditer une base de code, de concevoir un corpus de connaissances d'élite pour un langage (C++, Rust, TypeScript, Python...), et de le maintenir vivant au fil du projet.

---

## 1. Philosophie & Règle d'Or

> **« Le code IA produit ~2× plus de violations runtime que le code humain ; l'analyse statique seule est aveugle (arXiv / VulBench-CPP). La connaissance doit être explicite, typée et vérifiable. »**

Un corpus de connaissances pour agents ne doit **jamais** être un simple document encyclopédique passif. Il doit être :
1. **Actionnable :** Chaque document se termine par une checklist binaire (Oui/Non).
2. **Sélectif (Anti-dépassement de contexte) :** Structuré pour être chargé à la demande selon la tâche (Core, Domaine, Qualité).
3. **Citant et rétroactif :** Toute revue de code doit citer le corpus, et tout bug résolu doit enrichir le corpus.

---

## 2. Arborescence Cible d'un Corpus Langage

Pour tout langage ajouté dans `.harness/knowledge/languages/<lang>/` :

```
.harness/knowledge/languages/<lang>/
├── README.md                 Index, principes et matrice de chargement sélectif
├── core/                     ← SOCLE SYSTÉMATIQUE (lu pour toute tâche)
│   ├── ownership-raii.md     Gestion de la propriété, cycle de vie, passages de paramètres
│   ├── memory.md             Agencement cache (SoA/AoS), allocateurs/arenas, UB
│   ├── errors.md             Exceptions vs types résultats (expected/Result), politiques
│   ├── interfaces.md         Const-correctness, types forts, états invalides inexpressibles
│   └── modern-<lang>.md      Fonctionnalités modernes du standard (concepts, ranges, pattern matching...)
├── domains/                  ← CHARGÉ SELON LA TÂCHE
│   ├── scientific.md         Précision numérique, FMA/Kahan, vectorisation SIMD, convergence
│   ├── compilers.md          Lexer, Pratt parser, AST, gestion d'arènes, diagnostics, fuzzing
│   └── backend.md            Modèles de concurrence (mutex vs atomics), mémoire partagée
└── quality/                  ← CHARGÉ PAR LE REVIEWER
    ├── anti-patterns.md      Catalogue des anti-patterns interdits avec remèdes
    └── review-checklist.md   Grille d'évaluation (Bloquant / Majeur / Mineur) + outillage CI
```

---

## 3. Procédure d'Initialisation à partir d'une Codebase Réelle

### Étape 1 : Audit de la Codebase Hôte
Avant d'écrire le corpus, l'agent inspecte le projet existant :
1. Identifier la version du compilateur/runtime (`clang --version`, `rustc --version`, standard C++20/C++23).
2. Vérifier les outils d'analyse statique déjà branchés (`clang-tidy`, `clippy`, sanitizers).
3. Analyser les historiques d'erreurs (rapports de crash, commits de fix mémoire) pour recenser les pièges réels de l'équipe.

### Étape 2 : Rédaction du Socle (`core/`)
Rédiger les 5 fichiers indispensables :
- Spécifier sans ambiguïté les règles d'ownership (ex: *aucun pointeur brut propriétaire*).
- Dresser la liste exacte des comportements indéterminés (*Undefined Behaviors*) critiques.
- Établir la règle stricte de gestion des erreurs (ex: `std::expected` aux frontières internes, exceptions pour les fautes irrécupérables).

### Étape 3 : Spécialisation par Domaine (`domains/`)
- Rédiger les chapitres spécifiques uniquement si le projet les exploite (calcul scientifique, moteur de compilation, infrastructure réseau).
- Donner des seuils chiffrés : tolérance d'erreur numérique ($10^{-12}$), débit de parsing requis, ordres mémoire atomiques obligatoires.

### Étape 4 : Définition de la Grille de Review (`quality/`)
- Établir la liste des **anti-patterns formellement interdits**.
- Fournir une checklist de relecture avec grille de sévérité :
  - **Bloquant (Score 0) :** UB, fuite mémoire ASan, race condition, pointeur pendant.
  - **Majeur (-2) :** Allocation inutile dans une boucle critique, copie non intentionnelle.
  - **Mineur (-1) :** Absence de `[[nodiscard]]` ou de type fort explicite.

---

## 4. Matrice de Chargement Sélectif (Conservation du Contexte)

Pour éviter de saturer la fenêtre de contexte de l'IA lors des invites, l'agent applique scrupuleusement la table de routage définie dans le `README.md` du langage :

| Rôle / Tâche de l'Agent | Fichiers du Corpus à charger |
|---|---|
| **Développeur (Tâche standard)** | `core/*` uniquement |
| **Développeur (Calcul scientifique / SIMD)** | `core/*` + `domains/scientific.md` |
| **Développeur (Lexer / Parser / Compilateur)** | `core/*` + `domains/compilers.md` |
| **Développeur (Concurrence / Réseau)** | `core/*` + `domains/backend.md` |
| **Reviewer / Contrôleur Qualité** | `quality/*` (+ le domaine concerné) |

---

## 5. Maintien et Boucle de Rétroaction Continue

Le corpus n'est pas gravé dans le marbre : c'est une base de connaissances vivante.

1. **La règle de la review citante :**
   Lorsqu'un agent effectue une review, chaque rejet ou remarque doit citer explicitement le document de référence (ex: *« Violation de core/ownership-raii.md §3 »*).
2. **La règle de l'enrichissement post-incident :**
   Si un bug mémoire (ASan), un dead-lock (TSan) ou un résultat incohérent passe à travers les mailles du filet :
   - Corriger le code.
   - **Obligation immédiate :** Ajouter l'anti-pattern exact et sa détection dans `quality/anti-patterns.md`.
3. **Alignement Terrain $\rightarrow$ Connaissance :**
   Si une pratique réelle du code s'avère plus performante que la règle écrite, le document du corpus est révisé et validé par un benchmark reproductible.
