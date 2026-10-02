# Core React : Pureté de Rendu & Hooks Idiomatiques

> Ce document établit les règles d'or du moteur de rendu React et de l'utilisation rigoureuse des hooks.

---

## 1. La Pureté de Rendu

Un composant React doit être strictement déterministe :
- Même entrée (`props`, `state`) $\implies$ même sortie JSX.
- **Interdiction formelle :** Modifier des variables globales, lancer des requêtes réseau ou muter le DOM directement pendant la phase de rendu.

---

## 2. Règle Absolue du Tableau de Dépendances

Ne **JAMAIS** mentir à React sur les dépendances d'un hook (`useEffect`, `useMemo`, `useCallback`) :
- Toute variable ou fonction utilisée dans le hook doit figurer dans le tableau de dépendances.
- Si une dépendance change trop souvent et déclenche l'effet en boucle :
  - Extraire la fonction en dehors du composant.
  - Ou utiliser `useCallback` sur la fonction dépendante.
  - Ou décomposer l'effet.

---

## 3. Quand Utiliser `useMemo` et `useCallback` ?

1. **Ne pas mémoriser par défaut :** Mémoriser a un coût (allocation de tableau, comparaison de dépendances).
2. **Cas où `useMemo` est obligatoire :**
   - Calculs mathématiques coûteux (ex: filtrage de 10 000 éléments, inversion de matrice).
   - Objets de configuration passés en props à des composants enfants enveloppés dans `React.memo`.
3. **Cas où `useCallback` est obligatoire :**
   - Fonctions passées en props à des composants enfants optimisés ou placées dans les dépendances d'un `useEffect`.

---

## 4. Checklist Actionnable pour l'Agent

- [ ] Les composants de rendu sont-ils des fonctions pures sans effets de bord pendant l'évaluation JSX ?
- [ ] Aucun avertissement de `react-hooks/exhaustive-deps` n'est-il désactivé ?
- [ ] La logique métier complexe est-elle extraite dans des hooks personnalisés (`useComputeJob`, `useSimulationData`) ?
