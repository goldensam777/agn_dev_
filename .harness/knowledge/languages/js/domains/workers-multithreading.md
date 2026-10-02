# Domaine JS : Multithreading, Workers & Mémoire Partagée (`SharedArrayBuffer`)

> Ce document établit les règles d'exécution parallèle en JavaScript.

---

## 1. Web Workers & Node.js `worker_threads`

- Le thread JavaScript principal doit rester 100% dédié à la réactivité de l'interface ou au traitement des requêtes HTTP.
- Tout calcul numérique, simulation ou tâche lourde doit être envoyé à un pool de Workers.

---

## 2. Mémoire Partagée Réelle via `SharedArrayBuffer` & `Atomics`

Pour éviter la sérialisation entre threads, plusieurs Workers peuvent partager le même buffer de mémoire physique :

```javascript
// Allocation d'un buffer partagé (non copié, accessible par tous les threads)
const sharedBuffer = new SharedArrayBuffer(1024);
const sharedInts = new Int32Array(sharedBuffer);

// Synchronisation sans verrou avec les opérations atomiques V8
Atomics.add(sharedInts, 0, 1); // Incrémentation atomique sûre entre threads

// Attente et notification (comme une variable de condition POSIX)
// Atomics.wait(sharedInts, index, expectedValue); // Sur un worker uniquement
// Atomics.notify(sharedInts, index, count);
```

---

## 3. Checklist Actionnable pour l'Agent

- [ ] Les calculs intensifs sont-ils déportés sur un Worker ?
- [ ] Les accès concurrents sur `SharedArrayBuffer` passent-ils systématiquement par `Atomics` ?
- [ ] Aucun appel à `Atomics.wait()` n'est exécuté sur le thread principal (interdit par les navigateurs) ?
