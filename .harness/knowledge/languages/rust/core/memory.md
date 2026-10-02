# Core Rust : Agencement Mémoire, Allocateurs & Cache

> Ce document traite de l'optimisation physique de la mémoire en Rust, de l'élimination du padding et des stratégies d'allocation avancées.

---

## 1. Agencement Mémoire (`repr`) et Alignement

Par défaut, Rust applique `#[repr(Rust)]` : le compilateur réordonne librement les champs d'une structure pour minimiser le gaspillage d'octets (*padding*).

### Règles d'Attributs `repr` :
- **`#[repr(C)]` :** Obligatoire pour toute structure traversant la frontière FFI vers C++ ou C. Préserve l'ordre exact des champs.
- **`#[repr(align(64))]` :** Utilisé pour aligner une structure sur une ligne de cache processeur (64 octets) afin d'éliminer le *false sharing* en environnement multithread.
- **`#[repr(packed)]` : DANGER CRITIQUE.** N'utiliser que sous contrainte de protocole réseau/binaire strict. Déréférencer un champ non aligné d'une structure packed est un **Undefined Behavior (UB)** immédiat en Rust.

---

## 2. Stack vs Heap : Maîtrise des Tailles

1. **Tailles connues à la compilation (`Sized`) :**
   - Toujours privilégier l'allocation sur la pile (*stack*).
   - Les types récursifs (comme un AST : `enum Expr { Binary(Box<Expr>, Box<Expr>) }`) doivent obligatoirement utiliser un indirection `Box<T>` pour avoir une taille finie.
2. **`Box<[T]>` vs `Vec<T>` (Le réflexe d'économie mémoire) :**
   - Un `Vec<T>` stocke 3 mots pointeurs : `(ptr, capacity, len)` soit 24 octets sur 64-bit.
   - Si un vecteur n'a plus vocation à grandir une fois construit, le convertir avec `vec.into_boxed_slice()`. Un `Box<[T]>` ne stocke que `(ptr, len)` soit 16 octets, libérant le surplus de capacité.

---

## 3. Allocateurs d'Arènes (`bumpalo` vs `typed-arena`)

Pour les structures de données denses (AST de compilateur, graphes d'équations, simulateurs) :
- Les allocations individuelles sur le tas (`Box::new(...)`) fragmentent la mémoire et détruisent l'efficacité du cache L1/L2.
- **La solution industrielle :** L'allocation par bloc contigu en arène.

| Outil | Usage recommandé | Comportement `Drop` |
|---|---|---|
| **`bumpalo`** | Arbre syntaxique (AST), graphes d'analyse temporaires | **Zéro appel à `Drop`** individuel : libération globale en bloc en fin de passe de compilation ($O(1)$). |
| **`typed-arena`** | Objets homogènes nécessitant un nettoyage explicite | Exécute le destructeur `Drop` pour chaque élément lors de la destruction de l'arène. |

---

## 4. Ordonnancement Manuel des Champs

Bien que Rust optimise `repr(Rust)`, pour les structures `#[repr(C)]`, ordonner toujours les champs par ordre de taille décroissante pour éviter les trous d'alignement :
```rust
// ❌ MAUVAIS (12 octets utilisés, 4 octets de padding gaspillés)
#[repr(C)]
struct Bad {
    a: u8,   // 1 octet (+ 3 octets de padding)
    b: u32,  // 4 octets
    c: u8,   // 1 octet (+ 3 octets de padding)
}

// ✓ BON (8 octets, zéro padding)
#[repr(C)]
struct Good {
    b: u32,  // 4 octets
    a: u8,   // 1 octet
    c: u8,   // 1 octet (+ 2 octets de fin si alignement u32)
}
```

---

## 5. Checklist Actionnable pour l'Agent

- [ ] Les structures FFI sont-elles toutes marquées `#[repr(C)]` ?
- [ ] Aucun déréférencement direct de référence vers un champ `#[repr(packed)]` ?
- [ ] Les collections en lecture seule après construction utilisent-elles `Box<[T]>` ou `Arc<[T]>` ?
- [ ] Les graphes/arborescences volumineuses utilisent-ils une arène (`bumpalo`) au lieu d'un océan de `Box` ou `Rc` ?
