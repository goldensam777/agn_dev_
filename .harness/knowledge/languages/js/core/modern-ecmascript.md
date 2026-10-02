# Core JS : Fonctionnalités ECMAScript Modernes (ES2024 / ES2025)

> Ce document résume les API JavaScript récentes qui remplacent les bibliothèques tierces obsolètes.

---

## 1. `Promise.withResolvers()`

Élimine le pattern fastidieux de promesse différée (*deferred*) :
```javascript
// ✓ NOUVEAU STANDARD UNIVERSEL :
const { promise, resolve, reject } = Promise.withResolvers();

// Utilisation immédiate dans des gestionnaires d'événements asynchrones
button.onclick = () => resolve("Action terminée !");
```

---

## 2. Méthodes de Tableaux Non Mutables

Permettent de manipuler des listes sans altérer les tableaux d'origine :
```javascript
const original = [3, 1, 2];

// Ne mute pas 'original' : renvoie une nouvelle copie triée
const sorted = original.toSorted();

// Remplace un élément à un indice sans muter
const updated = original.with(1, 42); // [3, 42, 2]
```

---

## 3. Champs Privés Natifs (`#champ`) & `structuredClone`

1. **Vraie Encapsulation Matérielle :**
   - Utiliser `#field` plutôt que `_field`. Les champs précédés de `#` sont invisibles pour `Object.keys()` et `Reflect`.
2. **Clonage Profond Standard (`structuredClone`) :**
   - Remplacement universel de `JSON.parse(JSON.stringify(x))` ou `lodash.cloneDeep`. Supporte les références circulaires, les `Map`, les `Set` et les `TypedArray`.

---

## 4. Checklist Actionnable pour l'Agent

- [ ] Les promesses différées utilisent-elles `Promise.withResolvers()` ?
- [ ] Les manipulations de listes immuables utilisent-elles `toSorted()`, `toReversed()`, `with()` ?
- [ ] Le clonage d'objets complexes utilise-t-il `structuredClone` plutôt que des hacks JSON ?
