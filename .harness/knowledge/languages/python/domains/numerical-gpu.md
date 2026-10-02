# Domaine Python : Accélération JIT, Calcul GPU & Tenseurs

> Accélération compilée à la volée avec JAX (`@jax.jit`), Numba (`@njit`) et PyTorch (`torch.compile`) sur GPU/TPU/CPU.

---

## 1. Vue d'Ensemble des Accélérateurs JIT

| Technologie | Cas d'Usage Idéal | Modèle de Programmation |
|---|---|---|
| **JAX** | Calcul scientifique pur, différentiation automatique, vectorisation massive | Fonctions pures, immuabilité, compilation XLA |
| **Numba** | Boucles numériques en Python pur, code bas-niveau sur CPU | Décorateur `@njit(fastmath=True, nogil=True)` |
| **PyTorch (`torch.compile`)** | Réseaux de neurones, tenseurs massifs sur GPU (CUDA / ROCm) | Graphe dynamique optimisé via TorchDynamo/Inductor |

---

## 2. Accélération JIT avec JAX

JAX compile le code Python vers le compilateur XLA (Accelerated Linear Algebra) pour une exécution ultra-rapide sur GPU ou CPU multithreadé :

```python
import jax
import jax.numpy as jnp

# Règle d'or JAX : La fonction doit être pure (aucun effet de bord, pas de mutation in-place)
@jax.jit
def step_simulation(state: jax.Array, dt: float) -> jax.Array:
    # Opérations vectorielles pures
    velocity = jnp.sin(state) * 0.5
    next_state = state + velocity * dt
    return next_state

# Vectorisation automatique sur une dimension par lot (batch)
batch_step = jax.vmap(step_simulation, in_axes=(0, None))
```

---

## 3. Accélération Bas-Niveau avec Numba

Lorsque l'écriture sous forme matricielle vectorisée est impossible ou moins lisible qu'une boucle explicite :

```python
import numba

@numba.njit(fastmath=True, nogil=True, cache=True)
def mandelbrot_pixel(c_real: float, c_imag: float, max_iter: int) -> int:
    z_real = 0.0
    z_imag = 0.0
    for i in range(max_iter):
        r2 = z_real * z_real
        i2 = z_imag * z_imag
        if r2 + i2 > 4.0:
            return i
        z_imag = 2.0 * z_real * z_imag + c_imag
        z_real = r2 - i2 + c_real
    return max_iter
```

> **Avantage :** `nogil=True` permet à Numba de relâcher totalement le GIL pendant l'exécution de la fonction compilée, permettant le multi-threading natif.

---

## 4. Règle Critique de Gestion Mémoire GPU : Résidence des Tenseurs

Le transfert de données entre la mémoire vive de l'hôte (RAM CPU) et la mémoire vidéo du GPU (VRAM) via le bus PCIe est le goulot d'étranglement numéro 1.

### Règle absolue :
- **Bannir :** Les transferts de tenseurs CPU <-> GPU à chaque itération d'une boucle (`tensor.cpu().numpy()` au milieu d'une boucle d'optimisation).
- **Exiger :** Garder l'ensemble des données et de l'état de simulation sur le GPU pendant toute la durée de la boucle de calcul.

---

## 5. Checklist Actionnable pour l'Agent

- [ ] Les fonctions compilées avec JAX sont-elles strictement pures (zéro mutation, zéro I/O) ?
- [ ] Les fonctions Numba utilisent-elles `nogil=True` et `cache=True` ?
- [ ] Y a-t-il zéro appel de transfert mémoire hôte/accélérateur (`.cpu()`, `.item()`) à l'intérieur des boucles critiques ?
