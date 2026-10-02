# Corpus de Connaissances C (C23) — Pour Agents de Développement

> Corpus de référence interne du `.harness/knowledge/languages/c/`. Encode la rigueur d'ingénierie système en C moderne (C23), sans fuite mémoire ni comportement indéterminé (*Zero UB*). **À charger sélectivement** : le `core/` systématiquement ; les `domains/` selon la tâche ; `quality/` pour le reviewer.

---

## 1. Structure du Corpus

```
c-knowledge/
├── README.md                 ← Ce fichier (index + règles de chargement)
├── core/                     ← SOCLE SYSTÉMATIQUE (lu pour toute tâche C)
│   ├── memory-arenas.md      Allocateurs arènes (bump/region), malloc/free discipliné, ownership
│   ├── pointers-ub.md        Strict provenance, arithmétique de pointeurs, catalogue des UB en C
│   ├── error-handling.md     Codes d'erreur typés, struct Result en C, goto cleanup pattern (RAII en C)
│   ├── interfaces-c23.md     Const-correctness, structures opaques (pImpl en C), [[nodiscard]], types stricts
│   └── modern-c23.md         Standard C23 : nullptr, constexpr, static_assert, typeof, attributs natifs
├── domains/                  ← CHARGÉ À LA DEMANDE selon la tâche
│   ├── systems.md            POSIX/Linux syscalls, I/O io_uring, descripteurs, mémoire mmap
│   ├── numerical.md          SIMD intrinsics, mot-clé restrict, aliasing vectoriel, FMA
│   └── abi-ffi.md            Stabilité C ABI, ponts vers Rust, C++ et Node.js
└── quality/                  ← CHARGÉ PAR LE REVIEWER
    ├── anti-patterns.md      25 anti-patterns interdits (strcpy, casts void*, arithmétique void*...)
    └── review-checklist.md   Checklist de review + recette outillage (ASan, UBSan, Valgrind, Clang-Tidy)
```

---

## 2. Règles de Chargement (Matrice de Routage)

| Situation / Rôle de l'Agent | Fichiers à charger |
|---|---|
| **Toute tâche C** | `core/*` (5 fichiers fondamentaux) |
| **Système bas niveau / Linux / Fichiers** | `core/*` + `domains/systems.md` |
| **Calcul numérique / SIMD / DSP** | `core/*` + `domains/numerical.md` |
| **Frontière C ABI / Liaison FFI** | `core/*` + `domains/abi-ffi.md` |
| **Phase de Review / Contrôle Qualité** | `quality/*` (+ le domaine concerné) |

---

## 3. Principes Transverses du C Moderne (C23)

1. **La fin de `malloc`/`free` individuel :** Privilégier les arènes de mémoire (*Arena Allocators* / *Region-based Memory*). Une arène alloue par blocs contigus et se libère d'un seul coup en fin de requête ou de calcul.
2. **Strict Provenance :** Ne jamais convertir arbitrairement un entier en pointeur sans provenance valide.
3. **Le pattern RAII en C (`goto cleanup`) :** Tout point de sortie d'une fonction allouant des ressources doit passer par une étiquette `cleanup` unique pour garantir la libération.
4. **Le standard C23 obligatoire :** Utiliser `nullptr` (et non `NULL` ou `0`), `bool`, `true`/`false`, `constexpr`, et les attributs standard `[[nodiscard]]`.
5. **Zero Warning / Zero UB :** Compilation obligatoire sous `-Wall -Wextra -Wpedantic -Werror -fsanitize=address,undefined`.
