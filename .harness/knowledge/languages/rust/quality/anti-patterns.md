# Quality Rust : Catalogue des 25 Anti-Patterns Interdits

> Tout code enfreignant l'une de ces 25 règles sera rejeté lors de la revue et du contrôle de qualité.

---

### Catégorie A : Propriété, Emprunt & Mémoire
1. **Le `.clone()` de facilité :** Cloner une structure volumineuse simplement pour faire taire le borrow checker. *Remède : restructurer les lifetimes ou passer une référence empruntée.*
2. **Types d'arguments trop rigides :** Prendre `&String` ou `&Vec<T>` en paramètre. *Remède : prendre `&str` ou `&[T]`.*
3. **Allocation sans pré-dimensionnement :** Créer un `Vec::new()` dans une boucle sans `Vec::with_capacity(n)`.
4. **Concaténation de chaînes dans une boucle :** Utiliser `format!` ou l'opérateur `+` dans une boucle. *Remède : réutiliser un buffer `String` avec `write!` ou `push_str`.*
5. **Fuite mémoire par cycle `Arc` :** Deux structures se référençant mutuellement avec `Arc<Mutex<T>>`. *Remède : casser le cycle avec `Weak<T>`.*
6. **Collection non compactée :** Conserver un `Vec<T>` dont la taille ne bougera plus sans le convertir en `Box<[T]>`.
7. **Hashage par défaut trop lent dans la hot loop :** Utiliser `std::collections::HashMap` (SipHash résistant aux attaques DoS) dans un algorithme interne. *Remède : utiliser `ahash` ou `rustc-hash`.*

### Catégorie B : Types, Abstractions & API
8. **Primitive Obsession :** Passer des identifiants `u64` ou des montants `f64` bruts. *Remède : Newtype avec `#[repr(transparent)]`.*
9. **Dispatch dynamique inutile :** Mettre `Box<dyn Trait>` dans une boucle critique. *Remède : Génériques avec dispatch statique.*
10. **RefCell abusif :** Utiliser `RefCell` pour compenser une mauvaise conception de propriété.
11. **Drapeaux booléens d'état :** Utiliser des booléens pour gérer une machine à états. *Remède : Pattern Typestate.*
12. **Ignorer une valeur `#[must_use]` :** Ne pas consommer un `Result` ou un itérateur.

### Catégorie C : Erreurs & Paniques
13. **`unwrap()` en production :** Faire un `.unwrap()` sur un composant applicatif ou bibliothèque.
14. **`expect()` sans justification d'invariant :** Utiliser `.expect("failed")` sans expliquer pourquoi l'état est garanti sain.
15. **`panic!` sur une entrée invalide :** Paniquer face à des données corrompues au lieu de renvoyer un `Result::Err`.
16. **Égalité stricte sur les flottants :** Utiliser `a == b` sur `f32`/`f64`. *Remède : comparaison avec epsilon ou `total_cmp`.*

### Catégorie D : Code `unsafe` & Richesse Matérielle
17. **Bloc `unsafe` non documenté :** Absence du commentaire formel `// SAFETY: ...`.
18. **Création d'un `&mut T` qui aliase :** Créer une référence exclusive alors qu'un autre pointeur ou référence active existe.
19. **Usage de `mem::uninitialized()` :** Fonction obsolète et génératrice d'UB immédiat. *Remède : `MaybeUninit<T>`.*
20. **Déréférencement de champ `#[repr(packed)]` :** Accès direct non aligné. *Remède : copier la valeur d'abord via `ptr::read_unaligned`.*
21. **Transmutation sans vérification de taille :** `mem::transmute` entre deux types de tailles ou d'alignements non identiques.

### Catégorie E : Concurrence & Asynchrone
22. **Bloquer l'exécuteur Tokio :** Lancer une boucle SIMD ou un appel disque synchrone dans une tâche `async`. *Remède : `tokio::task::spawn_blocking`.*
23. **Garder un `MutexGuard` à travers un `.await` :** Empêche d'autres tâches d'avancer et risque le deadlock.
24. **`Arc<Mutex<T>>` pour des scalaires :** Utiliser un mutex pour un entier ou un booléen. *Remède : `AtomicUsize`, `AtomicBool`.*
25. **`Ordering::SeqCst` aveugle :** Ralentit inutilement le bus mémoire. *Remède : synchronisation fine `Acquire`/`Release`.*

---

## Les 3 Questions de Review Magiques

Avant de valider un commit Rust, le reviewer se pose toujours ces 3 questions :
1. *« Si cette structure est clonée 10 000 fois, est-ce que nous détruisons le cache L1/L2 ? »*
2. *« Ce bloc unsafe peut-il déclencher une violation d'aliasing sous Miri avec Tree Borrows ? »*
3. *« Existe-t-il un moyen d'empêcher cette erreur à la compilation plutôt qu'au runtime ? »*
