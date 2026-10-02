# Core React : Cycle de Vie & Zéro Fuite Mémoire dans les Effets

> Ce document fixe les obligations de nettoyage de ressources pour éviter les fuites de mémoire dans l'interface.

---

## 1. La Règle d'Or : Tout Abonnement Exige sa Fonction de Nettoyage

Tout effet qui enregistre un écouteur, un timer ou une connexion réseau **doit obligatoirement retourner une fonction de désabonnement** :

```tsx
useEffect(() => {
  const handleResize = () => updateDimensions();
  window.addEventListener("resize", handleResize);

  // OBLIGATOIRE : Nettoyage lors du démontage du composant
  return () => {
    window.removeEventListener("resize", handleResize);
  };
}, []);
```

---

## 2. Annulation de Requêtes Réseau avec `AbortController`

Empêche les mises à jour d'état sur des composants démontés (*stale responses / memory leaks*) :

```tsx
useEffect(() => {
  const controller = new AbortController();

  async function loadData() {
    try {
      const res = await fetch("/api/compute/status", { signal: controller.signal });
      const data = await res.json();
      setStatus(data);
    } catch (err: any) {
      if (err.name !== "AbortError") {
        setError(err.message);
      }
    }
  }

  loadData();

  // Annule la requête en vol si l'utilisateur quitte la page
  return () => {
    controller.abort();
  };
}, []);
```

---

## 3. Checklist Actionnable pour l'Agent

- [ ] Tout `addEventListener` a-t-il son `removeEventListener` dans la fonction de retour de l'effet ?
- [ ] Tout `setInterval` / `setTimeout` est-il nettoyé avec `clearInterval` / `clearTimeout` ?
- [ ] Les requêtes HTTP asynchrones dans `useEffect` utilisent-elles `AbortController` ?
- [ ] Le code fonctionne-t-il sans warning sous `React.StrictMode` (double exécution en dev) ?
