"""Modèle canonique Python 3.12+ : calcul scientifique vectorisé, typé, reproductible.

Règles illustrées (corpus .harness/knowledge/languages/python/) :
- domains/scientific-numpy.md : PAS de boucle Python sur les données (100-500× plus
  lent) ; opérations vectorisées + broadcasting + vues (zéro copie).
- core/typing-mypy.md         : annotations complètes, propre sous mypy --strict.
- domains/scientific-numpy.md : reproductibilité — générateur seedé, déterminisme.

Vérification : mypy --strict canonical_vectorized_numpy.py
               python3 canonical_vectorized_numpy.py
"""

from typing import TypeAlias

import numpy as np
import numpy.typing as npt

FloatVector: TypeAlias = npt.NDArray[np.float64]


def pairwise_distances(points: FloatVector) -> FloatVector:
    """Matrice de distances euclidiennes (N, N).

    La double boucle Python naïve serait ~500× plus lente : ici la boucle vit
    dans le C de NumPy (SIMD + cache). Coût mémoire O(N²) — au-delà de N ~ 20k,
    préférer scipy.spatial.distance.cdist ou chunker les lignes.
    """
    if points.ndim != 2 or points.shape[1] != 3:
        raise ValueError(f"points doit être de forme (N, 3), reçu {points.shape}")
    diff = points[:, np.newaxis, :] - points[np.newaxis, :, :]  # (N, N, 3) par broadcasting
    return np.sqrt(np.einsum("ijk,ijk->ij", diff, diff))


def monte_carlo_pi(n: int, seed: int = 42) -> tuple[float, float]:
    """Estimation de π par Monte Carlo, avec intervalle de confiance 95 %.

    Déterministe pour `seed` fixé : même entrée → même sortie, partout.
    """
    if n <= 0:
        raise ValueError("n doit être positif")
    rng = np.random.default_rng(seed)
    pts = rng.random((n, 2), dtype=np.float64)
    inside = int(np.count_nonzero(np.einsum("ij,ij->i", pts, pts) <= 1.0))
    estimate = 4.0 * inside / n
    # Bernoulli : Var[p̂] = p(1-p)/n, et estimate = 4p → Var = estimate(4-estimate)/n
    stderr = float(np.sqrt(estimate * (4.0 - estimate) / n))
    return estimate, 1.96 * stderr


if __name__ == "__main__":
    rng = np.random.default_rng(0)
    cloud = rng.random((200, 3))

    dist = pairwise_distances(cloud)
    assert dist.shape == (200, 200)
    expected = float(np.linalg.norm(cloud[0] - cloud[1]))
    assert abs(dist[0, 1] - expected) < 1e-12

    pi_hat, ic95 = monte_carlo_pi(1_000_000)
    print(f"π ≈ {pi_hat:.6f} ± {ic95:.6f} (IC 95 %)")
    print("Modèle NumPy validé : vectorisé, typé, reproductible.")
