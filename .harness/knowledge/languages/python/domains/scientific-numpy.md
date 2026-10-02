# Domaine Python : Calcul Scientifique & Vectorisation NumPy

> Vectorisation matricielle, agencement mémoire C vs Fortran, gestion stricte des vues zéro-copie et stabilité numérique.

---

## 1. Principes de la Vectorisation Haute Performance

Toute boucle `for` itérant sur des scalaires en Python doit être bannie au profit d'opérations vectorielles NumPy exécutées en C sous-jacent avec vectorisation SIMD :

```python
import numpy as np

# INVALIDE : Boucle scalaire lente, interprétée en Python
def compute_distance_naive(points_a: np.ndarray, points_b: np.ndarray) -> np.ndarray:
    n = len(points_a)
    dists = np.empty(n, dtype=np.float64)
    for i in range(n):
        diff = points_a[i] - points_b[i]
        dists[i] = np.sqrt(np.dot(diff, diff))
    return dists

# VALIDE : Vectorisation globale SIMD zéro boucle Python
def compute_distance_vectorized(points_a: np.ndarray, points_b: np.ndarray) -> np.ndarray:
    diff = points_a - points_b
    return np.linalg.norm(diff, axis=1)
```

**Gain observé :** Typiquement 80x à 250x d'accélération.

---

## 2. Agencement Mémoire : C-Order vs Fortran-Order

L'agencement des éléments en mémoire conditionne directement l'efficacité du cache processeur (L1/L2) :
- **C-Order (Row-Major) :** Les lignes sont contiguës en mémoire. Parcourir par lignes (`axis=1`).
- **Fortran-Order (Column-Major) :** Les colonnes sont contiguës. Parcourir par colonnes (`axis=0`).

```python
# Vérifier la contiguïté mémoire avant une boucle critique ou un passage vers C++
matrix = np.zeros((1000, 1000), dtype=np.float64, order="C")
assert matrix.flags["C_CONTIGUOUS"], "La matrice doit être contiguë en C pour le code natif"

# Utiliser np.ascontiguousarray si une opération de slicing a rompu la contiguïté
view_transposed = matrix.T
contiguous_transposed = np.ascontiguousarray(view_transposed)
```

---

## 3. Vues Zéro-Copie vs Copies Cachées

Comprendre ce qui alloue une copie et ce qui crée une simple vue :
- **Vues Zéro-Copie :** Découpage standard (`arr[1:10:2]`), `arr.reshape(...)`, `arr.T`.
- **Copies Implicites :** Indexation avancée (*fancy indexing*) (`arr[[0, 2, 4]]`), masques booléens (`arr[arr > 0]`).

### Opérations en place avec `out=` :
Pour éviter d'allouer des tableaux temporaires intermédiaires lors d'opérations en chaîne :

```python
a = np.ones((5000, 5000), dtype=np.float64)
b = np.ones((5000, 5000), dtype=np.float64)
result = np.empty_like(a)

# Évite l'allocation d'un buffer temporaire géant
np.multiply(a, 2.5, out=result)
np.add(result, b, out=result)
```

---

## 4. Stabilité Numérique & Précision Flottante

Éviter l'annulation catastrophique (*catastrophic cancellation*) lors de soustractions de nombres proches ou de calculs d'exponentielles :

| Calcul Naïf Invalide | Fonction Numérique Stable | Raison |
|---|---|---|
| `np.log(1.0 + x)` | `np.log1p(x)` | Précision maximale lorsque `x -> 0` |
| `np.exp(x) - 1.0` | `np.expm1(x)` | Évite la perte de bits de précision près de 0 |
| `np.log(np.sum(np.exp(x)))` | `scipy.special.logsumexp(x)` | Prévient l'overflow et l'underflow d'exponentielles |

---

## 5. Checklist Actionnable pour l'Agent

- [ ] Aucune boucle `for` purement scalaire n'opère-t-elle sur des données matricielles ?
- [ ] Les tableaux passés à du code C/C++ natif sont-ils explicitement `C_CONTIGUOUS` ?
- [ ] Les calculs récurrents utilisent-ils le paramètre `out=` pour éliminer les allocations temporaires ?
- [ ] Les calculs sensibles aux arrondis utilisent-ils `log1p`, `expm1` ou `logsumexp` ?
