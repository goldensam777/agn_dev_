# Quality TS : Catalogue des 25 Anti-Patterns Interdits

> Tout code enfreignant l'une de ces 25 règles sera rejeté lors de la revue et du contrôle de qualité.

---

### Catégorie A : Perte de Sécurité de Typage
1. **L'usage de `any` :** Désactive le compilateur. *Remède : `unknown` + Zod ou type guard.*
2. **Le cast aveugle `as Type` :** Forcer un type sur des données réseau ou disque non vérifiées.
3. **L'assertion non-nulle `!` :** Écrire `data!.field` au lieu de tester formellement la présence de la valeur.
4. **Enums numériques TypeScript classiques :** Génèrent du code JavaScript verbeux et peu typé. *Remède : Union de chaînes ou objet `as const`.*
5. **Types `Function` ou `Object` :** Types trop larges acceptant n'importe quoi. *Remède : Signatures précises `() => void` ou `Record<string, unknown>`.*
6. **Bypass du compilateur avec `@ts-ignore` :** Interdit sauf dérogation absolue documentée avec `@ts-expect-error`.
7. **`any` déguisé (`Record<string, any>`) :** Tolérer des champs non typés.

### Catégorie B : Modélisation & Invariants
8. **Primitive Obsession :** Passer des identifiants `string` nus interchangeables. *Remède : Branded Types.*
9. **Duplication de schéma et d'interface :** Écrire manuellement `interface User` ET `const UserSchema`. *Remède : `type User = z.infer<typeof UserSchema>;`.*
10. **Omission d'exhaustivité (`assertNever`) :** Oublier le cas `default` dans un `switch` d'union discriminée.
11. **Index signature trop permissive :** Utiliser `[key: string]: string` quand les clés sont connues.
12. **Mutabilité accidentelle des paramètres :** Modifier en place un objet ou un tableau reçu en argument.

### Catégorie C : Asynchronisme & Promesses
13. **Promesses flottantes :** Oublier un `await` sur une promesse ou ne pas capturer ses erreurs.
14. **`try...catch` silencieux :** Capturer une exception sans la logger ni la transformer (`catch {}`).
15. **Bloquer l'Event Loop :** Effectuer du calcul lourd dans le thread Node.js principal.
16. **Ignorer les signaux d'arrêt système :** Ne pas écouter `SIGTERM` / `SIGINT`.

### Catégorie D : Performance & Moteur V8
17. **Suppression de propriété avec `delete` :** Désoptimise la classe cachée (*hidden class*) du moteur V8. *Remède : assigner `undefined` ou recréer l'objet.*
18. **Explosion combinatoire de types :** Unions de Template Literals générant des milliers de types internes.
19. **Comparaison non stricte `==` :** Utiliser `==` au lieu de `===`.
20. **Recréation d'objets constants dans les boucles :** Alloue de la mémoire inutilement.

### Catégorie E : Style & Maintenabilité
21. **Imports circulaires :** Provoquent des valeurs `undefined` silencieuses à l'exécution.
22. **Types au milieu des composants :** Doivent être centralisés dans `contracts/` ou un fichier de types dédié.
23. **Classes comme simples conteneurs de données :** Préférer des types purs et fonctions pures.
24. **`boolean` pour des états exclusifs :** Préférer les unions discriminées.
25. **`export default` massif :** Préférer les exports nommés (`named exports`) pour un refactoring automatisé fiable.
