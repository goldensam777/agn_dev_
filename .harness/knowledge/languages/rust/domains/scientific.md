# Domaine Rust : Calcul Scientifique, SIMD & Stabilité Numérique

> Ce document définit les standards de précision mathématique et de calcul haute performance en Rust.

---

## 1. Précision Flottante & Élimination de la Cancellation

Les calculs en virgule flottante (`f64`, `f32`) respectent la norme IEEE 754 mais sont sujets à la **cancellation catastrophique** (perte de chiffres significatifs lors de la soustraction de deux nombres très proches).

### Sommation Compensée de Kahan (Obligatoire pour les grandes réductions)
Ne jamais faire un simple `.iter().sum()` sur des millions de flottants de magnitudes disparates :
```rust
pub fn kahan_sum(slice: &[f64]) -> f64 {
    let mut sum = 0.0;
    let mut c = 0.0; // Compensation d'arrondi
    for &x in slice {
        let y = x - c;
        let t = sum + y;
        c = (t - sum) - y;
        sum = t;
    }
    sum
}
```

---

## 2. Vectorisation SIMD en 3 Paliers

| Palier | Technique | Quand l'utiliser |
|---|---|---|
| **Palier 1 : Auto-vectorisation** | Itérateurs contigus + `-C target-cpu=native` | Boucles simples (produit scalaire, transformation map). Aider LLVM en évitant les sauts de branches dans la boucle. |
| **Palier 2 : SIMD Portable (`core::simd`)** | Types vectoriels `f64x4`, `f64x8`, opérations par lane | Calculs vectoriels explicites multiplateformes (x86 AVX2/AVX-512, ARM Neon). |
| **Palier 3 : Bibliothèques pures (`faer`)** | `faer::mat`, factorisations LU/Cholesky/SVD | Calcul matriciel dense. **`faer` est l'étalon de référence en Rust** : écrit en pur Rust, multithreadé via Rayon, rivalise avec OpenBLAS sans problème de liaison C. |

### Exemple de Produit Scalaire `core::simd` :
```rust
#![feature(portable_simd)]
use std::simd::prelude::*;

pub fn dot_product_simd(a: &[f64], b: &[f64]) -> f64 {
    assert_eq!(a.len(), b.len());
    const LANES: usize = 4;
    let mut sum_vec = f64x4::splat(0.0);

    let chunks_a = a.chunks_exact(LANES);
    let chunks_b = b.chunks_exact(LANES);
    let remainder_a = chunks_a.remainder();
    let remainder_b = chunks_b.remainder();

    for (ca, cb) in chunks_a.zip(chunks_b) {
        let va = f64x4::from_slice(ca);
        let vb = f64x4::from_slice(cb);
        sum_vec += va * vb;
    }

    let mut total = sum_vec.reduce_sum();
    // Traitement du reliquat scalaire
    for (&x, &y) in remainder_a.iter().zip(remainder_b) {
        total += x * y;
    }
    total
}
```

---

## 3. Stratégie de Test & Convergence

- Ne jamais tester l'égalité exacte `assert_eq!(a, b)` sur des flottants.
- Toujours utiliser une tolérance relative ou absolue adaptée :
  ```rust
  pub fn assert_approx_eq(actual: f64, expected: f64, epsilon: f64) {
      let diff = (actual - expected).abs();
      assert!(
          diff <= epsilon,
          "Écart de précision intolérable : |{actual} - {expected}| = {diff} > {epsilon}"
      );
  }
  ```

---

## 4. Checklist Actionnable pour l'Agent

- [ ] Les grandes réductions de flottants utilisent-elles la sommation compensée (Kahan / Neumaier) ?
- [ ] L'auto-vectorisation a-t-elle été vérifiée ou le code utilise-t-il `core::simd` / `faer` ?
- [ ] Aucun test n'utilise-t-il `assert_eq!` sur des flottants ?
- [ ] Les matrices denses volumineuses délèguent-elles à `faer` plutôt qu'à des boucles manuelles imbriquées ?
