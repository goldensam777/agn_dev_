# Quality React : Catalogue des 25 Anti-Patterns Interdits

> Tout code enfreignant l'une de ces 25 règles sera rejeté lors de la revue et du contrôle de qualité.

---

### Catégorie A : Cycle de Vie & Fuites Mémoire
1. **Effet sans fonction de nettoyage :** Oublier de retourner `() => removeEventListener(...)` ou `cancelAnimationFrame`.
2. **Requête réseau sans `AbortController` dans un effet :** Provoque des mises à jour d'état sur un composant démonté.
3. **Tableau de dépendances menteur :** Omettre des variables utilisées dans `useEffect`/`useCallback` pour bloquer un re-render.
4. **`useEffect` pour synchroniser de l'état dérivé :** Remplacer par du calcul direct lors du rendu.
5. **Boucle infinie de re-renders :** Muter un état dans un `useEffect` qui dépend de ce même état sans condition d'arrêt.
6. **Stale Closure :** Accéder à une variable obsolète dans une fonction asynchrone non rafraîchie.

### Catégorie B : Performance & Rendu
7. **Rendu à 60 fps dans l'arbre React :** Mettre à jour du `state` à chaque frame pour une animation ou simulation WebGL.
8. **Rendu de 1 000+ nœuds DOM sans virtualisation :** Épuise le navigateur.
9. **Mémorisation compulsive inutile :** Mettre `useMemo` sur des calculs triviaux ($a + b$).
10. **Objets instanciés inline passés à des enfants mémorisés :** Casse la mémorisation de `React.memo`.
11. **Props drilling excessif :** Passer une prop à travers 6 composants intermédiaires passifs.
12. **Clefs instables dans les listes :** Utiliser `key={index}` ou `key={Math.random()}` pour des listes mutables.

### Catégorie C : État & Pureté
13. **Mutation directe de l'état :** `state.user.name = "Alice"` au lieu de passer un nouvel objet.
14. **Mise à jour non fonctionnelle concurrente :** `setCount(count + 1)` au lieu de `setCount(c => c + 1)`.
15. **Effet de bord dans le corps du composant :** Déclencher une mutation ou un appel API directement dans le rendu JSX.
16. **Store global utilisé pour de l'état purement local :** Polluer le contexte global.

### Catégorie D : Architecture & Typage
17. **Props non typées ou typées avec `any` :** Absence d'interface TypeScript stricte.
18. **Composant monolithique de 600 lignes :** Manque de découpage conteneur / présentation.
19. **Logique métier enfouie dans le JSX :** Doit être extraite dans un hook personnalisé (`use...`).
20. **Manipulation directe du DOM :** Utiliser `document.getElementById` au lieu de `useRef`.

### Catégorie E : Accessibilité & Ergonomie
21. **Boutons cliquables sans `button` :** Utiliser `<div onClick={...}>` sans rôle d'accessibilité ni gestion clavier.
22. **Images ou icônes sans attribut `alt` ou `aria-label` :** Inaccessible aux lecteurs d'écran.
23. **Absence de retour visuel sur les actions lentes :** Bouton non désactivé pendant un calcul.
24. **Contraste insuffisant sur les métriques clés :** Texte gris clair sur fond blanc illisible.
25. **Ignorer les avertissements en mode strict :** Ne pas tester avec `<React.StrictMode>`.
