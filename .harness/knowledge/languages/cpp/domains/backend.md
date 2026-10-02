# C++ Backend : Services, Concurrence & API C++

> **Idée directrice** : La qualité d'un backend se mesure sous charge et à l'échec : pas de
> fuite sur les requêtes longues, latence bornée (p99), erreurs qui remontent au bon
> niveau (cf. errors.md), et concurrence **prouvée** (TSan en CI) plutôt qu'espérée.
> Règle d'or : **mutex par défaut, lock-free uniquement sous preuve de besoin, jamais de
> partage mutable sans discipline.**

---

## 1. Le modèle de concurrence : choisir avant d'écrire

1. **Message passing / data ownership par thread** (recommandé) : chaque donnée mutable a
   un propriétaire thread ; les autres communiquent par files de messages (MPMC/SPSC).
   Minimalise les races par construction. Ex. : thread réseau → file → thread métier.
2. **Shared state + locks** : acceptable et simple tant que le verrou est court, rare, et
   la structure sous le verrou petite. La discipline : `std::lock_guard`/`scoped_lock`
   systématiques, jamais de `lock()`/`unlock()` manuels.
3. **Lock-free** : uniquement pour des structures précises sous contention mesurée. Coût :
   ABA, réclamation mémoire (hazard pointers, epoch), atomics et memory orders à justifier
   ligne par ligne. Une file lock-free battle-testée existe : `boost::lockfree::queue`.

```cpp
// ❌ double-checked locking à la main, ordering fragile, ABA…
// ✅ si vraiment nécessaire, prouver + documenter ; sinon :
std::shared_mutex cache_mutex;             // read-mostly : N readers OK
```

## 2. La boîte à outils concurrence (avec quand)

| Outil | Usage | Piège |
|---|---|---|
| `std::mutex` + `lock_guard` | Section critique courte | `lock()` manuel → deadlock/exception |
| `std::scoped_lock` | Plusieurs mutex sans deadlock ordonné | — |
| `std::shared_mutex` | Read-heavy (config, cache) | écrivains affamés : mesurer |
| `std::atomic<T>` | Compteurs, flags, publication unique | `relaxed` sans raison = bug latent |
| `std::condition_variable` | Producteur/consommateur, attente événement | lost wakeup si le wait n'est pas sous le même mutex que le notify-flag |
| `std::latch` / `barrier` (C++20) | Rendez-vous de phase | — |
| `std::jthread` + `stop_token` | Tâche annulable proprement | ne pas ignorer le stop : c'est un contrat |
| `std::async`/futures | Calculs parallèles à résultat | ne pas enchaîner les `.get()` bloquants sans timeout |
| Thread pool borné | Tout le reste | jamais `std::thread` par unité de travail |

### Memory orders : la réduction du dangereux
La règle de survie : **`acquire`/`release` pour publier des données (le pattern 99% des
cas), `seq_cst` si tu n'es pas sûr, `relaxed` seulement pour des statistiques où la
cohérence globale n'a aucune importance.** Toujours documenter pourquoi l'ordre choisi est
suffisant.

```cpp
// Publication : le pattern standard, à reconnaître et réutiliser tel quel
void publish(const Data& d) {
    compute_into_shared_buffer(d);                 // (1) écrits
    ready.store(true, std::memory_order_release);  // (2) clôture
}
void consume() {
    while (!ready.load(std::memory_order_acquire)) { }  // (3) synchronise avec (2)
    read_shared_buffer();                               // voit garanti les écrits (1)
}
```

## 3. Pièges de concurrence à connaître

- **Deadlock** : verrous imbriqués → `std::scoped_lock(m1, m2)` ; sinon ordre global
  documenté. Pas de callback utilisateur sous verrou.
- **Data race** : deux accès, dont une écriture, sans synchronisation → UB (pas "bug", UB :
  le compilateur peut optimiser au-delà de toute logique). TSan le détecte — l'exécuter en
  CI est un critère de qualité.
- **False sharing** : cf. memory.md — padding/alignas(64) sur les compteurs par thread.
- **Dangling en async** : lambda stockée/passée à un autre thread → capturer **par valeur**
  ou par `shared_ptr`/`weak_ptr` ; capturer `&` un local qui meurt = use-after-free
  (Core Guideline F.53).
- **Lifecycle des threads** : `join` avant la fin du scope qui possède les données qu'ils
  utilisent. `jthread` automatise.

## 4. Conception d'API C++ pour services

- **La frontière de bibliothèque est un contrat** : types propres (cf. interfaces.md),
  erreurs via exceptions/expected documentées (cf. errors.md), jamais de `exit`/`abort`/
  `cerr` côté bibliothèque.
- **Callbacks et asynchronisme** : un appel qui termine "plus tard" doit dire qui possède
  les données pendant ce temps. Préférer : handle annulable, completion token, ou
  `std::function` + ownership explicite.
- **Backpressure** : toute file non bornée est une fuite de mémoire en attente. Files
  bornées + politique (drop oldest / bloquer / rejeter) explicite.
- **Timeouts partout** : attente sur verrou, I/O, future. Une opération sans timeout est
  un deadlock différé.
- **Côté serveur** : une requête = un scope. Tout ce qui est alloué pour une requête meurt
  avec elle (arena par requête, cf. memory.md — le pattern protobuf). Aucune donnée
  inter-requêtes mutable sans verrou.

## 5. Sérialisation & I/O

- Ne jamais parser soi-même JSON/protobuf/etc. sauf raison majeure : bibliothèques
  éprouvées (nlohmann/json, simdjson, protobuf, FlatBuffers — binaire zéro-parse, arena-
  friendly).
- Validation stricte à la frontière : tailles, encodages, champs requis — *tout* input
  réseau est hostile par défaut.
- I/O async (io_uring, asio) : à envisager seulement quand le profil mesuré (beaucoup de
  connexions, I/O-bound) le justifie ; sinon le modèle thread-pool-bloqué est plus simple
  et suffisant.

## 6. Observabilité : la qualité en production

- Logs structurés (niveau, requête-id, durée), jamais dans les hot paths (per-request ok,
  par-élément non).
- Métriques : latence p50/p95/p99 (pas juste la moyenne — la moyenne ment), débit, erreurs
  par type, saturation (taille des files).
- Tracing de requête : un id propagé end-to-end.
- Health checks qui testent réellement les dépendances (cf. `health.sh` du harness).

## 7. Checklist backend

- [ ] Modèle de concurrence choisi et documenté (ownership par thread / locks / lock-free justifié)
- [ ] Pas de `lock()`/`unlock()` manuels ; multi-mutex via `scoped_lock`
- [ ] Aucune capture `[&]` de locals dans du code asynchrone (F.53)
- [ ] Memory orders `release`/`acquire` documentés pour chaque publication
- [ ] Files bornées + politique de surcharge ; timeouts sur toutes les attentes
- [ ] TSan job en CI (tests sous contention) ; jamais de "ça marche chez moi" en concurrence
- [ ] Scope par requête : aucune fuite inter-requêtes ; arena si lifetime homogène
- [ ] Métriques p99 + logs structurés avec requête-id
- [ ] Pas de thread créé à la volée dans les chemins de requête
