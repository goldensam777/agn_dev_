# Corpus de connaissances C++ — pour agents de développement

> Corpus de référence interne du `.harness/knowledge/languages/cpp/`. Chaque document encode
> les critères de qualité d'un développeur C++ senior, par domaine. **À charger
> sélectivement** : le `core/` systématiquement ; les `domains/` selon la tâche ; la
> `quality/` pour le reviewer.

## Structure

```
cpp-knowledge/
├── README.md                    ← ce fichier (index + règles de chargement)
├── core/                        ← SOCLE — lu pour toute tâche C++
│   ├── ownership-raii.md        Propriété, RAII, règle des 0/5, passage de paramètres
│   ├── memory.md                Cache, SoA/AoS, arenas/allocateurs, UB, alignement
│   ├── errors.md                Exceptions vs expected vs assert, garanties, politique par couche
│   ├── interfaces.md            Const-correctness, conception d'API, types forts
│   └── modern-cpp.md            C++17/20/23, concepts, ranges, idiomes (PImpl, CRTP…)
├── domains/                     ← CHARGÉ À LA DEMANDE selon le playbook
│   ├── scientific.md            Flottants, cancellation, Kahan, SIMD, parallélisme, convergence
│   ├── compilers.md             Lexer/parser/AST/IR, diagnostics, fuzzing, golden tests
│   └── backend.md               Concurrence (mutex/atomics/memory orders), services, observabilité
└── quality/                     ← CHARGÉ PAR LE REVIEWER
    ├── anti-patterns.md         30 anti-patterns interdits + leurs remèdes
    └── review-checklist.md      Checklist de review + le outillage exact de verify.sh/CI
```

## Règles de chargement (pour le routage par le lead/playbook)

| Situation | Charger |
|---|---|
| Toute tâche C++ | `core/*` (5 fichiers) |
| Tâche numérique/simulation | + `domains/scientific.md` |
| Tâche lexer/parser/langage | + `domains/compilers.md` |
| Tâche serveur/concurrence | + `domains/backend.md` |
| Phase de review | + `quality/*` |
| Écriture d'un nouveau module | core + domaine + (quality pour auto-contrôle) |

## Principes transverses du corpus

1. **L'ownership est visible dans les types** — jamais une convention implicite.
2. **La structure des données avant les algorithmes** — le cache décide des performances.
3. **Les erreurs sont des valeurs ou des exceptions délibérées** — jamais des conventions magiques.
4. **L'UB est traité comme du code mort** — sanitizers en CI, toujours.
5. **Chaque domaine a sa définition de la qualité** (scientifique : correct à 1e-12 ;
   compilateur : sémantique préservée + diagnostics ; backend : borné + prouvé sous TSan).
6. **La qualité se mesure** — benchmark avant optimisation, convergence pour le numérique,
   fuzzing pour les parseurs.
7. **Les outils sont un filet, pas un substitut** — multi-tier : tests + statique +
   sanitizers + fuzz.

## Mise à jour du corpus

- Toute découverte d'un défaut non couvert par ce corpus → ajouter la règle ici
  (knowledge.md → ce dossier), pas seulement corriger le code.
- Tout nouveau pattern validé en review → candidat pour `examples/`.
- Fraîcheur : si le code réel diverge d'un document `core/`, c'est le document qui se
  corrige (règle inscrite dans CONVENTIONS.md).
