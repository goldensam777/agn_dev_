# Core TS : Rétrécissement de Types, Gardes & Exhaustivité

> Ce document fixe les règles d'élimination de l'ambiguïté de type à l'exécution sans casting forcé.

---

## 1. Unions Discriminées : L'Outil Maître de Modélisation

Toujours utiliser une propriété littérale discriminante commune (ex: `type`, `kind`, `status`) pour modéliser des états exclusifs :

```typescript
export type AsyncState<T> =
  | { status: "idle" }
  | { status: "loading" }
  | { status: "success"; data: T }
  | { status: "error"; error: Error };

// TypeScript sait exactement quels champs existent selon le 'status'
function renderState<T>(state: AsyncState<T>) {
  switch (state.status) {
    case "idle": return "En attente";
    case "loading": return "Chargement...";
    case "success": return `Résultat : ${state.data}`;
    case "error": return `Erreur : ${state.error.message}`;
    default: return assertNever(state);
  }
}
```

---

## 2. La Fonction de Vérification d'Exhaustivité (`assertNever`)

Garantit à la compilation qu'aucun nouveau cas ajouté à une union n'a été oublié dans un `switch` ou `if` :

```typescript
export function assertNever(x: never): never {
  throw new Error(`Cas non géré détecté au runtime : ${JSON.stringify(x)}`);
}
```
*Si vous ajoutez `{ status: "cancelled" }` à `AsyncState`, le `switch` refusera immédiatement de compiler.*

---

## 3. Gardes de Types Personnalisés (`is` & `asserts`)

Ne jamais utiliser `as Type` pour forcer un type inconnu :
```typescript
// Type Guard sûr
export function isComputationResult(val: unknown): val is { summaryValue: number } {
  return (
    typeof val === "object" &&
    val !== null &&
    "summaryValue" in val &&
    typeof (val as any).summaryValue === "number"
  );
}

// Fonction d'assertion
export function assertNonNull<T>(val: T | null | undefined, msg: string): asserts val is T {
  if (val === null || val === undefined) {
    throw new Error(msg);
  }
}
```

---

## 4. Checklist Actionnable pour l'Agent

- [ ] Les états complexes (chargement, requêtes, jobs) utilisent-ils des unions discriminées ?
- [ ] Tout `switch` sur une union discriminée se termine-t-il par `assertNever(variable)` dans le `default` ?
- [ ] Aucun opérateur d'assertion non-nulle `!` n'est-il utilisé sans garde préalable ?
