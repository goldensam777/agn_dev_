# Core JS : Boucle d'Événements (Event Loop & Libuv)

> Ce document établit la hiérarchie de traitement de l'Event Loop et prévient la famine (*starvation*) des tâches.

---

## 1. La Hiérarchie : Microtâches vs Macrotâches

À chaque tour de boucle (*tick*) :
1. V8 traite le code synchrone courant.
2. **Puis vide INTÉGRALEMENT la file des Microtâches :**
   - `process.nextTick()` (priorité maximale Node.js)
   - Résolutions de Promesses (`Promise.then()`, `await`)
   - `queueMicrotask()`
3. **Puis passe aux Macrotâches :**
   - Timers (`setTimeout`, `setInterval`)
   - Événements d'I/O réseau et fichiers
   - `setImmediate()` (Node.js)

---

## 2. Le Piège de la Famine de Microtâches (*Microtask Starvation*)

Si une microtâche résout continuellement d'autres promesses en boucle, l'Event Loop **ne passera jamais** aux entrées/sorties réseau ni aux timers :
```javascript
// ❌ CRASH SILENCIEUX : La boucle d'événements est bloquée à 100%, l'I/O est mort de faim !
function starve() {
  Promise.resolve().then(starve);
}
```

### Le Remède : Céder la Main (*Yielding to the Loop*)
Dans un traitement lourd par lots, céder explicitement la main à la boucle après chaque tranche de 10 ms :
```javascript
export function yieldToEventLoop() {
  return new Promise((resolve) => setImmediate(resolve));
}
```

---

## 3. Checklist Actionnable pour l'Agent

- [ ] Les traitements par lots volumineux incluent-ils des points de cession (*yield*) avec `setImmediate` ?
- [ ] Aucun enchaînement récursif infini de microtâches (`process.nextTick` ou promesses immédiates) ?
- [ ] Les I/O lentes sont-elles toutes non-bloquantes ?
