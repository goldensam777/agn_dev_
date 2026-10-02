# Corpus de Connaissances JavaScript (V8 & Runtime) — Pour Agents de Développement

> Corpus de référence interne du `.harness/knowledge/languages/js/`. Encode l'optimisation extrême sous le moteur V8, le zéro-copie avec les `ArrayBuffer` et la gestion rigoureuse de la boucle d'événements. **À charger sélectivement** : le `core/` systématiquement ; les `domains/` selon la tâche ; `quality/` pour le reviewer.

---

## 1. Structure du Corpus

```
js-knowledge/
├── README.md                    ← Ce fichier (index + règles de chargement)
├── core/                        ← SOCLE SYSTÉMATIQUE (lu pour toute tâche JS)
│   ├── v8-internals.md          Classes cachées (Shapes), Inline Caches (IC), déoptimisations JIT
│   ├── memory-gc.md             Garbage Collector (Scavenger vs Mark-Sweep), fuites par fermetures
│   ├── event-loop-libuv.md      Microtâches (Promise) vs Macrotâches (I/O, timer), famine de boucle
│   ├── typed-arrays-buffers.md  ArrayBuffer, TypedArrays, zéro-copie et transfert de propriété
│   └── modern-ecmascript.md     ES2024/ES2025 : structuredClone, Promise.withResolvers, Temporal
├── domains/                     ← CHARGÉ À LA DEMANDE selon la tâche
│   ├── workers-multithreading.md Web Workers, worker_threads, SharedArrayBuffer & Atomics
│   ├── streams-io.md            Streams Node & Web Streams, gestion de la contre-pression (backpressure)
│   └── jit-optimization.md      Optimisation TurboFan : tableaux denses SMI vs HeapNumbers
└── quality/                     ← CHARGÉ PAR LE REVIEWER
    ├── anti-patterns.md         25 anti-patterns interdits (delete, polymorphisme excessif, trous dans tableaux)
    └── review-checklist.md      Checklist de review + flags de profilage Node (--trace-deopt, --trace-gc)
```

---

## 2. Règles de Chargement (Matrice de Routage)

| Situation / Rôle de l'Agent | Fichiers à charger |
|---|---|
| **Toute tâche JavaScript / Runtime** | `core/*` (5 fichiers fondamentaux) |
| **Parallélisme / Multithreading / SharedMemory** | `core/*` + `domains/workers-multithreading.md` |
| **I/O massif / Fichiers / Flux continus** | `core/*` + `domains/streams-io.md` |
| **Optimisation de boucles critiques (Hot Loops)** | `core/*` + `domains/jit-optimization.md` |
| **Phase de Review / Contrôle Qualité** | `quality/*` (+ le domaine concerné) |

---

## 3. Principes Transverses de JavaScript Haute Performance

1. **Stabilité des Formes d'Objets (*Hidden Classes / Shapes*) :** Toujours initialiser les propriétés des objets dans le même ordre et dans le constructeur pour éviter la déoptimisation des caches en ligne (Inline Caches).
2. **Bannir le mot-clé `delete` :** Supprimer une clé avec `delete obj.prop` détruit la classe cachée de l'objet et le rétrograde en mode dictionnaire lent (*slow dictionary mode*). Assigner `undefined` ou recréer l'objet.
3. **Zéro-Copie via `ArrayBuffer` :** Pour manipuler des flux binaires ou scientifiques, utiliser exclusivement des `Float64Array` / `Uint8Array` et transférer leur possession sans copie via la liste de transfert (*transferable objects*).
4. **Priorité absolue à la Boucle d'Événements :** Ne jamais laisser une boucle synchrone monopoliser le thread pendant plus de 16 ms (seuil d'une frame à 60 fps).
