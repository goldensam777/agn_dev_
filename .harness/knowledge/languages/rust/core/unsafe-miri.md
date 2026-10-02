# Core Rust : Code `unsafe`, Modèles d'Emprunt & Validation sous Miri

> Ce document établit le protocole le plus strict du dépôt : la gestion du code `unsafe`.

---

## 1. Les 5 Super-Pouvoirs et la Règle du Contrat

Le mot-clé `unsafe` ne désactive **pas** le borrow checker. Il déverrouille uniquement 5 actions :
1. Déréférencer un pointeur brut (`*const T`, `*mut T`).
2. Appeler une fonction ou une fonction FFI marquée `unsafe`.
3. Implémenter un trait marqué `unsafe` (comme `Send` ou `Sync`).
4. Mutater ou accéder à une variable `static mut`.
5. Accéder aux champs d'une `union`.

**Règle absolue :** Tout bloc `unsafe` doit être précédé d'un commentaire formel débutant par `// SAFETY:` démontrant point par point pourquoi les préconditions sont garanties. Tout `unsafe` non commenté entraîne un **rejet immédiat**.

---

## 2. Modèles d'Emprunt : Stacked Borrows vs Tree Borrows

Pour dériver du code machine optimal, LLVM émet l'attribut `noalias` sur les références exclusives `&mut T`. Si deux pointeurs bruts violent cette exclusivité, le compilateur produit du code machine erroné en silence.

### Les Modèles de Vérification :
- **Stacked Borrows (Historique) :** Gère les permissions sous forme de pile d'éléments empruntés. Très strict, rejette certains motifs d'emprunts disjoints légitimes.
- **Tree Borrows (Moderne & Standard actuel) :** Organise les permissions en arborescence hiérarchique avec des *protecteurs* de fonction. Permet des découpages fins de buffers tout en interdisant formellement l'aliasing illégal.

---

## 3. Mémoire Non Initialisée : Obligation `MaybeUninit<T>`

Il est strictement interdit d'utiliser des motifs obsolètes pour allouer de la mémoire non initialisée.
```rust
use std::mem::MaybeUninit;

// ✓ BON : Allocation sécurisée de buffer sans pénalité d'initialisation
let mut buffer: [MaybeUninit<u8>; 1024] = unsafe {
    MaybeUninit::uninit().assume_init() // Interdit !
}; // ❌ Non !

// ✓ LA VRAIE MÉTHODE SÛRE :
let mut buffer: [MaybeUninit<u8>; 1024] = [const { MaybeUninit::uninit() }; 1024];

// Remplissage...
// Une fois TOUT le buffer écrit, et SEULEMENT ALORS :
let initialized = unsafe {
    // SAFETY: Nous garantissons que les 1024 octets ont été écrits avant conversion.
    std::mem::transmute::<[MaybeUninit<u8>; 1024], [u8; 1024]>(buffer)
};
```

---

## 4. Protocole de Validation sous Miri

Tout composant contenant au moins un bloc `unsafe` doit impérativement être validé sous l'interpréteur Miri avant soumission :

```bash
# Vérification sous le modèle moderne Tree Borrows
MIRIFLAGS="-Zmiri-tree-borrows" cargo miri test
```

Miri traque à l'exécution :
- Les accès hors limites.
- Les déréférencements de pointeurs nuls ou mal alignés.
- Les violations d'aliasing (écrire via un pointeur alors qu'une référence `&` existe).
- Les fuites de mémoire.

---

## 5. Checklist Actionnable pour l'Agent

- [ ] Tout bloc `unsafe` possède-t-il un commentaire `// SAFETY:` exhaustif ?
- [ ] Aucune référence `&mut` n'est-elle créée à partir d'un pointeur qui pourrait aliaser une autre référence active ?
- [ ] La mémoire non initialisée utilise-t-elle strictement `MaybeUninit<T>` ?
- [ ] Le code `unsafe` a-t-il été validé avec `MIRIFLAGS="-Zmiri-tree-borrows" cargo miri test` avec 0 erreur ?
