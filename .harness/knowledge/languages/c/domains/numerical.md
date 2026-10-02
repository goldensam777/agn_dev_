# Domaine C : Calcul Numérique, SIMD & Mot-Clé `restrict`

> Ce document fixe les techniques d'optimisation vectorielle et mathématique de pointe en C.

---

## 1. Le Mot-Clé `restrict` : Libérer la Vectorisation

En C, si deux pointeurs pointent vers le même type, le compilateur doit assumer qu'ils peuvent se chevaucher en mémoire (*aliasing*), ce qui désactive la plupart des vectorisations SIMD.

En ajoutant `restrict`, vous certifiez formellement qu'aucun autre pointeur n'accède à cette plage mémoire :
```c
// Sans restrict : le compilateur recharge *out à chaque tour de boucle (aliasing potentiel)
// Avec restrict : vectorisation automatique avec registres AVX/Neon
void vector_add(size_t n, 
                const double* restrict a, 
                const double* restrict b, 
                double* restrict out) 
{
    for (size_t i = 0; i < n; ++i) {
        out[i] = a[i] + b[i];
    }
}
```

---

## 2. Intrinsics SIMD AVX2 & Alignement Matériel

Pour les noyaux de calcul critiques ne pouvant être confiés à l'auto-vectoriseur :
- Aligner les buffers sur 32 octets (AVX2) ou 64 octets (AVX-512) :
  ```c
  alignas(32) double buffer[256];
  ```
- Utiliser les intrinsics dédiés avec `fma` :
  ```c
  #include <immintrin.h>

  // Chargement et FMA vectoriel 256-bit (4 doubles d'un coup)
  __m256d va = _mm256_load_pd(&a[i]);
  __m256d vb = _mm256_load_pd(&b[i]);
  __m256d acc = _mm256_fmadd_pd(va, vb, acc);
  ```

---

## 3. Checklist Actionnable pour l'Agent

- [ ] Les fonctions de calcul vectoriel exploitent-elles le mot-clé `restrict` sur leurs pointeurs de données ?
- [ ] Les buffers traités par instructions vectorielles sont-ils explicitement alignés (`alignas(32)`) ?
- [ ] Le drapeau `-march=native` ou `-mavx2` est-il spécifié pour les bancs de performance ?
