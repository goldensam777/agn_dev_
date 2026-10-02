# Quality React : Grille de Revue & Recette Outillage Multi-Tier

> Ce document fournit la grille d'évaluation et la recette outillage appliquée pour les interfaces React.

---

## 1. Grille d'Évaluation & Système de Scoring

### Niveau 1 : Défauts Bloquants (Score 0 — Rejet Immédiat)
- Fuite mémoire avérée dans un effet (écouteur d'événement, timer ou canvas sans cleanup).
- Mutation directe de l'état ou des props.
- Boucle infinie de re-renders déclenchée par un `useEffect`.
- Effet de bord non encapsulé dans la phase de rendu d'un composant.

### Niveau 2 : Défauts Majeurs (Pénalité -2 par occurrence)
- Utilisation de `useEffect` pour synchroniser de l'état dérivé au lieu d'un calcul direct.
- Rendu de plus de 100 éléments sans virtualisation.
- Absence d'utilisation d'`AbortController` sur les requêtes asynchrones déclenchées dans les effets.
- Omission de dépendances dans un hook signalée par le linter.

### Niveau 3 : Défauts Mineurs (Pénalité -1 par occurrence)
- Clef d'index `key={index}` sur une liste dynamique.
- Absence de police monospace sur l'affichage de métriques numériques critiques.
- Omission de `aria-label` sur un bouton d'icône.

---

## 2. Règle de la Review Citante

Chaque commentaire de rejet de code doit citer formellement le document et paragraphe correspondant :
- Exemple : *"Rejet : Bloquant selon `.harness/knowledge/languages/react/core/lifecycle-cleanup.md §1` (l'écouteur 'resize' dans le canvas n'est pas nettoyé lors du démontage du composant)."*

---

## 3. Recette Outillage Multi-Tier (Pour scripts/verify.sh)

```bash
# Tier 1 : Vérification stricte des types React
npx tsc --noEmit

# Tier 2 : Tests unitaires de composants
npm test

# Tier 3 : Linter React Hooks & Accessibilité
npx eslint src/ --max-warnings 0
```
