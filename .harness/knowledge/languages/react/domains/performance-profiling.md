# Domaine React : Profilage de Performance & Virtualisation de Données

> Ce document établit les méthodes d'élimination des goulots d'étranglement de rendu en React.

---

## 1. Virtualisation des Listes et Tableaux Massifs

- Ne **JAMAIS** rendre 1 000 éléments DOM simultanément dans une liste ou un tableau de métriques.
- **La règle de virtualisation :** Rendre uniquement les éléments visibles à l'écran plus un petit buffer de débordement (via `@tanstack/react-virtual` ou `react-window`).
- L'empreinte mémoire reste constante quel que soit le nombre de lignes (10 000 lignes ont le même coût DOM que 20 lignes).

---

## 2. Détection des Re-renders Inutiles

Utiliser les outils de profilage React DevTools pour identifier :
- Les composants qui re-rendent alors que leurs props n'ont pas changé structurellement.
- L'instanciation de nouveaux objets inline dans les props (`style={{ ... }}`, `onClick={() => ...}`) passées à des composants mémorisés.

---

## 3. Checklist Actionnable pour l'Agent

- [ ] Les listes de données volumineuses (> 100 éléments) sont-elles virtualisées ?
- [ ] Les composants de rendu intensifs sont-ils protégés par `React.memo` avec comparaison shallow ?
- [ ] Aucun calcul lourd n'est-il réexécuté sans `useMemo` lors de re-renders fréquents ?
