# Quality JS : Catalogue des 25 Anti-Patterns Interdits

> Tout code enfreignant l'une de ces 25 règles sera rejeté lors de la revue et du contrôle de qualité.

---

### Catégorie A : Performance JIT & V8
1. **L'usage du mot-clé `delete` :** Détruit la classe cachée V8. *Remède : assigner `undefined`.*
2. **Création de tableaux à trous (*Holey Arrays*) :** `arr[100] = x` sur un tableau vide.
3. **Mélange de types dans les tableaux :** Mettre des entiers, des chaînes et des objets dans un même tableau critique.
4. **Appels de fonctions mégamorphiques :** Passer plus de 4 formes d'objets distinctes à une même fonction chaude.
5. **Modification de l'ordre d'initialisation des propriétés :** Brise le partage des Shapes V8.
6. **Modification du prototype à l'exécution :** `Object.setPrototypeOf()` déoptimise tout le code lié.
7. **Usage de `eval()` ou `new Function()` :** Désactive totalement les optimisations du compilateur TurboFan.

### Catégorie B : Boucle d'Événements & Concurrence
8. **Bloquer l'Event Loop avec du calcul synchrone :** Boucles CPU sans cession (*yield*).
9. **Famine de microtâches :** Boucle récursive de `Promise.then` ou `process.nextTick`.
10. **`Atomics.wait` sur le thread principal :** Fige le navigateur ou Node.js.
11. **Copie complète de buffer vers un worker :** Oublier la liste de transfert (*transferable list*).
12. **Ignorer la contre-pression (*backpressure*) dans les flux :** Lire plus vite qu'on ne peut écrire.

### Catégorie C : Mémoire & Garbage Collector
13. **Fuite mémoire par closure inutile :** Retenir un gros buffer dans une fonction parente.
14. **`Map` globale au lieu de `WeakMap` :** Empêche le GC de libérer des instances mortes.
15. **Timer oublié sans `clearInterval` :** Conserve le composant en mémoire indéfiniment.
16. **Allocations massives dans la boucle chaude :** Créer des objets temporaires à chaque itération.
17. **Utiliser `slice()` au lieu de `subarray()` :** Copie inutile de mémoire sur un `TypedArray`.

### Catégorie D : Fiabilité & ECMAScript
18. **Comparaison non stricte `==` :** Risque de coercition de type implicite farfelue.
19. **Promesse sans `catch()` :** Déclenche un `unhandledRejection`.
20. **`JSON.parse(JSON.stringify(x))` pour cloner :** Remplacer par `structuredClone(x)`.
21. **Champs privés par convention `_name` :** Remplacer par `#name` natif.
22. **Création manuelle de promesses différées :** Remplacer par `Promise.withResolvers()`.
23. **Mutation d'arguments de fonction :** Modifie l'état de l'appelant sans prévenir.
24. **`for...in` sur un tableau :** Plus lent que `for...of` ou une boucle indicée standard.
25. **Ignorer les erreurs dans un bloc `catch` vide :** Masque les pannes système.
