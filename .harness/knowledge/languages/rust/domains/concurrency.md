# Domaine Rust : Concurrence, Parallélisme & Modèles Mémoire

> Ce document fixe les règles d'ingénierie multithread et de mémoire partagée en Rust.

---

## 1. Les Piliers Fondamentaux : `Send` et `Sync`

- **`Send` :** Indique qu'un type peut transférer sa propriété d'un thread à un autre en toute sécurité.
- **`Sync` :** Indique qu'une référence `&T` peut être partagée entre plusieurs threads simultanés sans *data race*. Formule clé : `T: Sync` $\iff$ `&T: Send`.
- **Types non `Sync` / non `Send` critiques :**
  - `Rc<T>` et `RefCell<T>` ne sont ni `Send` ni `Sync` (utiliser `Arc<T>` et `parking_lot::Mutex<T>`).
  - Les pointeurs bruts `*const T` et `*mut T` ne sont ni `Send` ni `Sync`.

---

## 2. Parallélisme de Données CPU avec `rayon`

Pour tout calcul numérique, vectorisation d'itérateurs ou traitement par lots CPU-bound :
- Ne jamais manipuler de `std::thread::spawn` manuellement.
- Utiliser **`rayon`** et son ordonnanceur à vol de travail (*work-stealing*) :
```rust
use rayon::prelude::*;

// Transformation parallèle sans allocation superflue
let results: Vec<f64> = large_dataset
    .par_iter()
    .map(|&x| compute_heavy_function(x))
    .collect();
```

---

## 3. Asynchrone (`tokio`) : Interdiction de Bloquer l'Exécuteur

Dans un serveur backend asynchrone :
- **Règle absolue :** Ne JAMAIS exécuter de calcul lourd (chiffrement, boucle SIMD, décompression) ou d'I/O synchrone (`std::fs`, `std::net`) au milieu d'une tâche `async`.
- **Remède obligatoire :** Déléguer au pool de threads bloquants :
  ```rust
  let result = tokio::task::spawn_blocking(move || {
      heavy_simd_computation(&data)
  }).await?;
  ```

---

## 4. Primitives de Synchronisation & Ordres Mémoire Atomiques

1. **Préférer `parking_lot::Mutex` :**
   - Plus rapide, 1 seul octet d'état (contre 40+ octets pour `std::sync::Mutex` sous Linux), et ne souffre pas du mécanisme d'empoisonnement (*poisoning*).
2. **Ordres Mémoire Atomiques (`std::sync::atomic::Ordering`) :**
   - **`Relaxed` :** Compteurs simples ou statistiques où l'ordre des autres écritures n'a aucune importance.
   - **`Release` (écrivain) / `Acquire` (lecteur) :** Synchronisation formelle de structures sans verrou (*lock-free*).
   - **`SeqCst` :** Cohérence séquentielle globale. Ne pas utiliser par défaut : coûteux sur architectures ARM et x86.

---

## 5. Checklist Actionnable pour l'Agent

- [ ] Les calculs CPU intensifs utilisent-ils `rayon` plutôt que des threads instanciés manuellement ?
- [ ] Aucune opération bloquante (I/O synchrone ou boucle de calcul) ne s'exécute-t-elle dans le runtime `tokio` sans `spawn_blocking` ?
- [ ] Les verrous utilisent-ils `parking_lot` plutôt que `std::sync::Mutex` pour les chemins critiques ?
- [ ] Les opérations atomiques évitent-elles `SeqCst` sauf justification formelle ?
- [ ] Le code multithreadé complexe passe-t-il sous ThreadSanitizer (`-Zsanitizer=thread`) ?
