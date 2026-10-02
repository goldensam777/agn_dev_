# Core React : Rendu Concurrent & Nouveautés React 19

> Ce document traite des fonctionnalités modernes de React 19 pour fluidifier l'interface lors de calculs intensifs.

---

## 1. Priorisation des Mises à Jour avec `useTransition`

Lorsqu'un calcul scientifique ou un filtrage massif prend du temps, empêcher le gel du champ de saisie utilisateur en marquant la mise à jour comme non urgente :

```tsx
import { useState, useTransition } from "react";

export function SimulationControl() {
  const [isPending, startTransition] = useTransition();
  const [params, setParams] = useState(defaultParams);

  const handleSliderChange = (newVal: number) => {
    // 1. Mise à jour immédiate du curseur visuel (Haute priorité)
    setInputValue(newVal);

    // 2. Calcul lourd de mise à jour du graphique (Basse priorité, interruptible)
    startTransition(() => {
      setParams((prev) => ({ ...prev, dimension: newVal }));
    });
  };

  return (
    <div>
      <input type="range" onChange={(e) => handleSliderChange(Number(e.target.value))} />
      {isPending && <span>Recalcul de la simulation...</span>}
      <SimulationGraph params={params} />
    </div>
  );
}
```

---

## 2. Gestion Asynchrone Propre avec `useActionState`

Remplace l'enchaînement fastidieux de 3 `useState` (`data`, `isLoading`, `error`) :
```tsx
import { useActionState } from "react";

async function submitJobAction(previousState: any, formData: FormData) {
  try {
    const res = await submitComputeJob(...);
    return { data: res, error: null };
  } catch (err: any) {
    return { data: null, error: err.message };
  }
}

export function JobForm() {
  const [state, formAction, isPending] = useActionState(submitJobAction, { data: null, error: null });

  return (
    <form action={formAction}>
      <button type="submit" disabled={isPending}>
        {isPending ? "Exécution C++ en cours..." : "Lancer le calcul"}
      </button>
      {state.error && <p className="error">{state.error}</p>}
    </form>
  );
}
```

---

## 3. Checklist Actionnable pour l'Agent

- [ ] Les mises à jour visuelles lourdes (graphes, listes) sont-elles enveloppées dans `startTransition` ?
- [ ] Les formulaires asynchrones exploitent-ils `useActionState` plutôt que des cascades de `useState` manuels ?
- [ ] Les suspensions de données exploitent-elles des limites de secours `<Suspense fallback={...}>` explicites ?
