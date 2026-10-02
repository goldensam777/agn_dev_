# Core React : Gestion d'État & Dérivation de Données

> Ce document établit la gestion d'état minimale et prévisible en React.

---

## 1. Co-localisation de l'État : Principe du Moindre Périmètre

- Placer l'état au plus près du composant qui en a besoin.
- Ne remonter l'état (*lift state up*) vers un parent que si deux composants frères ont réellement besoin de le partager.
- Ne pas introduire de store global (Zustand/Redux) pour un simple état de formulaire ou de modalité d'affichage.

---

## 2. Le Bogue Majeur de l'État Dupliqué

Ne JAMAIS synchroniser un état avec un autre état via `useEffect` :
```tsx
// ❌ MAUVAIS (Re-render inutile, risque de désynchronisation)
const [items, setItems] = useState<Item[]>([]);
const [itemCount, setItemCount] = useState<number>(0);

useEffect(() => {
  setItemCount(items.length);
}, [items]);

// ✓ BON (Calcul dérivé à la volée pendant le rendu)
const [items, setItems] = useState<Item[]>([]);
const itemCount = items.length; // 0 re-render, 0 bug de désynchronisation !
```

---

## 3. Mises à Jour Fonctionnelles pour Éviter les Écrasements

Dès qu'un nouvel état dépend de la valeur précédente, utiliser la forme fonctionnelle :
```tsx
// ❌ Risque de stale state en cas d'appels asynchrones concurrents
setJobCount(jobCount + 1);

// ✓ Garanti à 100% sans race condition
setJobCount((prev) => prev + 1);
```

---

## 4. Checklist Actionnable pour l'Agent

- [ ] L'état est-il co-localisé au plus proche de son utilisation ?
- [ ] Aucune valeur dérivable n'est-elle stockée inutilement dans un `useState` ?
- [ ] Les mises à jour dépendantes de l'état précédent utilisent-elles la forme fonctionnelle `prev => ...` ?
