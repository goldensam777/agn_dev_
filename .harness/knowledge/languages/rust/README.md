# Corpus de Connaissances Rust — Pour Agents de Développement

> Corpus de référence interne du `.harness/knowledge/languages/rust/`. Encode les critères de qualité d'un ingénieur Rust senior / compiler engineer. **À charger sélectivement** : le `core/` systématiquement ; les `domains/` selon la tâche ; `quality/` pour le reviewer.

---

## 1. Structure du Corpus

```
rust-knowledge/
├── README.md                    ← Ce fichier (index + règles de chargement)
├── core/                        ← SOCLE — Lu pour toute tâche Rust
│   ├── ownership-borrowing.md   Lifetimes, borrow checker, aliasing XOR mutabilité, move semantics
│   ├── memory.md                Agencement mémoire, repr(C/packed/align), bumpalo/arenas, drop
│   ├── errors.md                Result/Option, thiserror vs anyhow, politique de panic, unwrap
│   ├── traits-types.md          Type-driven design, newtype, PhantomData, traits, dyn vs impl
│   └── unsafe-miri.md           Rustonomicon, Tree Borrows / Stacked Borrows, MaybeUninit, Miri CI
├── domains/                     ← CHARGÉ À LA DEMANDE selon le playbook
│   ├── scientific.md            core::simd, faer/nalgebra, auto-vectorisation, calcul IEEE 754
│   ├── compilers.md             Lexing Logos, Pratt parser, AST en arène bumpalo, diagnostics miette
│   └── concurrency.md           Send/Sync, Rayon (data parallelism), Tokio, Crossbeam, atomics
└── quality/                     ← CHARGÉ PAR LE REVIEWER
    ├── anti-patterns.md         25 anti-patterns interdits (clone compulsif, RefCell abusif, etc.)
    └── review-checklist.md      Checklist de review + recette outillage (clippy pedantic, miri)
```

---

## 2. Règles de Chargement (Matrice de Routage)

| Situation / Rôle de l'Agent | Fichiers à charger |
|---|---|
| **Toute tâche Rust** | `core/*` (5 fichiers fondamentaux) |
| **Calcul numérique / SIMD / Simulation** | `core/*` + `domains/scientific.md` |
| **Compilateur / Lexer / Parser / AST / DSL** | `core/*` + `domains/compilers.md` |
| **Backend / Concurrence / Asynchrone** | `core/*` + `domains/concurrency.md` |
| **Phase de Review / Contrôle Qualité** | `quality/*` (+ le domaine concerné) |
| **Écriture d'un nouveau crate/module** | `core/*` + domaine ciblé + `quality/review-checklist.md` |

---

## 3. Principes Transverses du Corpus Rust

1. **Rendre les états invalides inexpressibles :** Utiliser le système de types (enum, newtype, typestates) pour éliminer les bugs à la compilation.
2. **Aliasing XOR Mutabilité :** Une ressource peut avoir plusieurs références immuables (`&T`) OU une seule référence exclusive (`&mut T`), jamais les deux en même temps.
3. **Zéro allocation cachée :** Éviter les `.clone()` réflexes. Préférer le passage de références empruntées (`&str`, `&[T]`) et les allocateurs d'arènes (`bumpalo`) pour les graphes ou ASTs.
4. **Le code `unsafe` est un contrat formel :** Tout bloc `unsafe` doit être documenté avec un invariant `// SAFETY: ...` rigoureux et validé sous **Miri avec Tree Borrows**.
5. **Erreurs typées aux frontières de bibliothèques :** Utiliser `thiserror` pour les bibliothèques et modules internes, `anyhow` pour les applications ou orchestrateurs finaux. Jamais de `unwrap()` en production.
6. **Clippy Pedantic est la norme :** Le code doit compiler sans le moindre avertissement sous `cargo clippy -- -D clippy::all -D clippy::pedantic`.
