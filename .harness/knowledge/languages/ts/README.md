# Corpus de Connaissances TypeScript — Pour Agents de Développement

> Corpus de référence interne du `.harness/knowledge/languages/ts/`. Encode le typage statique avancé, la programmation au niveau des types (*type-level programming*) et la validation sans faille aux frontières. **À charger sélectivement** : le `core/` systématiquement ; les `domains/` selon la tâche ; `quality/` pour le reviewer.

---

## 1. Structure du Corpus

```
ts-knowledge/
├── README.md                    ← Ce fichier (index + règles de chargement)
├── core/                        ← SOCLE SYSTÉMATIQUE (lu pour toute tâche TS)
│   ├── type-system.md           Types conditionnels, mappés, inférence avancée, generics
│   ├── narrowing-guards.md      Unions discriminées, type guards custom, exhaustivité never
│   ├── branded-types.md         Typage nominal par branding (UserId, Dollar, Radian) sans surcoût
│   ├── contracts-validation.md  Zéro-trust aux frontières (Zod/ArkType), bannissement du 'as'
│   └── performance.md           Performance du compilateur tsc, évitement de l'explosion d'unions
├── domains/                     ← CHARGÉ À LA DEMANDE selon la tâche
│   ├── fullstack-ipc.md         RPC bout-en-bout, WebSocket typé, pont sérialisé vers le C++
│   ├── state-modeling.md        Immutabilité structurelle, modélisation d'état sans redondance
│   └── server-runtime.md        Node.js ESM, cycle de vie des promesses, unhandled rejections
└── quality/                     ← CHARGÉ PAR LE REVIEWER
    ├── anti-patterns.md         25 anti-patterns interdits (any, as Type, assert !, enum TS...)
    └── review-checklist.md      Checklist de review + recette tsc strict & ESLint type-checked
```

---

## 2. Règles de Chargement (Matrice de Routage)

| Situation / Rôle de l'Agent | Fichiers à charger |
|---|---|
| **Toute tâche TypeScript** | `core/*` (5 fichiers fondamentaux) |
| **Communication API / WebSocket / Bridge** | `core/*` + `domains/fullstack-ipc.md` |
| **Gestion d'état complexe / Store** | `core/*` + `domains/state-modeling.md` |
| **Backend Node.js / Serveur HTTP** | `core/*` + `domains/server-runtime.md` |
| **Phase de Review / Contrôle Qualité** | `quality/*` (+ le domaine concerné) |

---

## 3. Principes Transverses du TypeScript d'Élite

1. **Le typage est un outil de preuve formelle :** Utiliser les unions discriminées et les types conditionnels pour éliminer les bugs d'état à la compilation.
2. **Zéro `any`, Zéro casting sauvage (`as Type`) :** `any` désactive silencieusement le compilateur. Utiliser `unknown` et rétrécir avec des type guards ou des validateurs Zod.
3. **Le principe Zero-Trust aux frontières :** Toute donnée provenant du réseau, du stockage ou d'un appel IPC natif est non typée (`unknown`) jusqu'à validation par un schéma Zod (`contracts/`).
4. **Typage nominal via Branded Types :** Empêcher les confusions d'arguments numériques ou de chaînes via des marques de types à coût d'exécution nul.
5. **Vérification exhaustive (`never`) :** Tout switch sur une union discriminée doit posséder une branche `default: assertUnreachable(x);`.
