# Corpus de Connaissances React (React 19) — Pour Agents de Développement

> Corpus de référence interne du `.harness/knowledge/languages/react/`. Encode l'excellence des interfaces utilisateur réactives, du rendu concurrent React 19 et des visualisations scientifiques haute fidélité (WebGL/WebGPU). **À charger sélectivement** : le `core/` systématiquement ; les `domains/` selon la tâche ; `quality/` pour le reviewer.

---

## 1. Structure du Corpus

```
react-knowledge/
├── README.md                    ← Ce fichier (index + règles de chargement)
├── core/                        ← SOCLE SYSTÉMATIQUE (lu pour toute tâche React)
│   ├── rendering-hooks.md       Pureté des composants, hooks personnalisés, useMemo/useCallback
│   ├── state-management.md      Gestion d'état local vs global, dérivation d'état, immutabilité
│   ├── concurrent-react19.md    React 19 : useActionState, useOptimistic, transitions, Suspense
│   ├── lifecycle-cleanup.md     Zéro fuite mémoire dans useEffect : AbortController, cleanup
│   └── component-architecture.md Séparation stricte UI/Logique, Compound Components, typage de props
├── domains/                     ← CHARGÉ À LA DEMANDE selon la tâche
│   ├── scientific-viz.md        Intégration Canvases WebGL/WebGPU à 60fps sans bloquer React
│   ├── design-system.md         Hiérarchie visuelle, design tokens, responsive dashboard scientifique
│   └── performance-profiling.md Élimination des re-renders inutiles, virtualisation de gros volumes
└── quality/                     ← CHARGÉ PAR LE REVIEWER
    ├── anti-patterns.md         25 anti-patterns interdits (useEffect pour état dérivé, stale closures...)
    └── review-checklist.md      Checklist de review UI + tests Vitest & React Testing Library
```

---

## 2. Règles de Chargement (Matrice de Routage)

| Situation / Rôle de l'Agent | Fichiers à charger |
|---|---|
| **Toute tâche React** | `core/*` (5 fichiers fondamentaux) |
| **Graphes / Simulation / Canvas 3D / WebGPU** | `core/*` + `domains/scientific-viz.md` |
| **Création de composants UI / Thème** | `core/*` + `domains/design-system.md` |
| **Optimisation de latence de rendu / Lags** | `core/*` + `domains/performance-profiling.md` |
| **Phase de Review / Contrôle Qualité** | `quality/*` (+ le domaine concerné) |

---

## 3. Principes Transverses de React 19

1. **La pureté de rendu est un axiome :** Un composant React doit être une fonction pure de ses `props` et de son `state`. Zéro effet de bord pendant le calcul du JSX.
2. **Ne jamais utiliser `useEffect` pour des calculs dérivés :** Si une valeur peut être déduite de l'état existant ou des props, la calculer directement ou via `useMemo`.
3. **Nettoyage systématique des effets :** Tout écouteur d'événement, abonnement WebSocket, ou requête réseau lancé dans un effet doit posséder sa fonction de nettoyage (`cleanup`) avec `AbortController`.
4. **Isoler les boucles à 60 fps :** Les animations graphiques et rendus WebGL ne doivent jamais faire re-rendre l'arbre de composants React. Utiliser des `ref` vers les canvases HTML5 et des boucles `requestAnimationFrame` imperméables à React.
