# Répertoire d'Exemples de Référence (.harness/examples/)

Ce dossier contient des implémentations de référence et des patrons architecturaux
validés. Tout agent d'IA peut s'inspirer de ces modèles pour concevoir de nouveaux
modules sans réinventer les bonnes pratiques.

**Règle d'utilisation** : chaque exemple cite en en-tête les documents du corpus
qu'il illustre — lire l'exemple ET les documents référencés avant d'écrire du code
nouveau. Un bon exemple annoté vaut dix pages de règles.

## Contenu

| Fichier | Langage | Ce qu'il démontre | Corpus illustré |
|---|---|---|---|
| `canonical_vector_core.cpp` | C++20 | Calcul haute performance : RAII, cache, ASan | cpp/core/* |
| `canonical_pratt_parser_arena.cpp` | C++20 | Lexer zéro-copie + Pratt + arène bump | cpp/core/memory.md, cpp/domains/compilers.md |
| `canonical_kahan_summation.cpp` | C++20 | Précision numérique MESURÉE (Kahan/Neumaier) | cpp/domains/scientific.md |
| `canonical_lexer_pratt.rs` | Rust 2021 | Lexer + Pratt + arène chunkée (std-only, tests) | rust/core/*, rust/domains/compilers.md |
| `canonical_arena_c23.c` | C23 | Arène malloc, alignement explicite, propriété unique | c/core/* |
| `canonical_branded_contracts.ts` | TS strict | Types brandés + Zod aux frontières | ts/core/* |
| `canonical_scientific_canvas.tsx` | React 19 | WebGL isolé, cleanup intégral, zéro fuite GPU | react/domains/, react/core/lifecycle-cleanup.md |
| `canonical_worker_pipeline.js` (+ `.worker.js`) | ES2024 | Pool de workers, Transferable zéro-copie, backpressure | js/domains/workers-multithreading.md |
| `canonical_vectorized_numpy.py` | Python 3.12+ | NumPy vectorisé, typé, reproductible | python/domains/scientific-numpy.md |

## Protocole d'ajout d'un nouvel exemple

1. Compile et s'exécute sous les flags de CONVENTIONS.md (`-Werror`, `clippy -D warnings`,
   `tsc --strict`, ASan/UBSan, `rustc --test`, `mypy --strict`).
2. Cite ses règles en en-tête (références croisées vers le corpus).
3. Comporte des golden tests / asserts vérifiant le comportement.
4. Une fois déposé ici, l'ajouter au tableau `REQUIRED_FILES` de `scripts/verify.sh`
   (étape 5) — le juge le protège dès lors contre toute régression.
