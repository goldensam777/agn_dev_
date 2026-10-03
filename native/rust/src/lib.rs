//! # Module Natif Rust Haute Performance (`forge-math-core`)
//!
//! Ce module constitue l'équivalent Rust certifié de `native/include/math_core.hpp`.
//! Il applique les principes stricts de la Forge :
//! - Zéro allocation dynamique cachée sur le chemin critique de calcul.
//! - Déroulage de boucles SIMD-friendly et respect de la localité de cache.
//! - Gestion explicite des erreurs via le type standard [`Result`].

#![deny(missing_docs)]
#![deny(unsafe_code)]

use std::fmt;

/// Erreurs mathématiques et structurelles émises par le module.
#[derive(Debug, Clone, PartialEq, Eq)]
pub enum MathError {
    /// Incompatibilité de dimension entre deux vecteurs ou matrices.
    DimensionMismatch {
        /// Taille attendue.
        expected: usize,
        /// Taille observée.
        found: usize,
    },
    /// La taille du tampon linéaire ne correspond pas à une matrice carrée $N \times N$.
    InvalidMatrixDimension {
        /// Taille théorique attendue ($N \times N$).
        expected_len: usize,
        /// Taille effective du tampon.
        actual_len: usize,
    },
    /// Taille nulle non autorisée pour cette opération.
    ZeroDimension,
}

impl fmt::Display for MathError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Self::DimensionMismatch { expected, found } => {
                write!(
                    f,
                    "Incompatibilité de dimensions : attendu {expected}, reçu {found}"
                )
            }
            Self::InvalidMatrixDimension {
                expected_len,
                actual_len,
            } => {
                write!(
                    f,
                    "Taille de tampon invalide pour matrice carrée : attendu {expected_len}, reçu {actual_len}"
                )
            }
            Self::ZeroDimension => write!(f, "La dimension ne peut pas être nulle"),
        }
    }
}

impl std::error::Error for MathError {}

/// Calcule le produit scalaire $\sum_{i=0}^{N-1} a_i \times b_i$ à haute vitesse.
///
/// L'implémentation repose sur un itérateur fusionné (`zip`) favorisant la vectorisation
/// automatique (AVX / Neon) par LLVM sans allocation intermédiaire.
///
/// # Arguments
///
/// * `a` - Premier vecteur de réels 64-bits
/// * `b` - Second vecteur de réels 64-bits
///
/// # Erreurs
///
/// Renvoie [`MathError::DimensionMismatch`] si `a.len() != b.len()`.
///
/// # Exemples
///
/// ```
/// use forge_math_core::dot_product;
///
/// let a = [1.0, 2.0, 3.0];
/// let b = [4.0, 5.0, 6.0];
/// assert_eq!(dot_product(&a, &b).unwrap(), 32.0);
/// ```
pub fn dot_product(a: &[f64], b: &[f64]) -> Result<f64, MathError> {
    if a.len() != b.len() {
        return Err(MathError::DimensionMismatch {
            expected: a.len(),
            found: b.len(),
        });
    }

    let sum = a.iter().zip(b.iter()).map(|(&x, &y)| x * y).sum();
    Ok(sum)
}

/// Générateur pseudo-aléatoire léger et déterministe (Xorshift64).
struct XorShift64 {
    state: u64,
}

impl XorShift64 {
    fn new(seed: u64) -> Self {
        // Garantir un état non nul
        Self {
            state: if seed == 0 { 0x5a17_d00d } else { seed },
        }
    }

    fn next_f64(&mut self) -> f64 {
        let mut x = self.state;
        x ^= x << 13;
        x ^= x >> 7;
        x ^= x << 17;
        self.state = x;
        // Conversion vers l'intervalle [0.0, 1.0)
        (x >> 11) as f64 * (1.0 / (1u64 << 53) as f64)
    }
}

/// Estime la constante $\pi$ par intégration Monte Carlo probabiliste.
///
/// Simule le tirage uniforme de points dans un carré $[-1, 1] \times [-1, 1]$
/// et compte la proportion tombant dans le disque unité $x^2 + y^2 \le 1$.
///
/// # Arguments
///
/// * `iterations` - Nombre total de tirages
/// * `seed` - Graine initiale du générateur pseudo-aléatoire
pub fn monte_carlo_pi(iterations: usize, seed: u64) -> f64 {
    if iterations == 0 {
        return 0.0;
    }

    let mut rng = XorShift64::new(seed);
    let mut inside = 0usize;

    for _ in 0..iterations {
        let x = rng.next_f64() * 2.0 - 1.0;
        let y = rng.next_f64() * 2.0 - 1.0;
        if x * x + y * y <= 1.0 {
            inside += 1;
        }
    }

    4.0 * (inside as f64) / (iterations as f64)
}

/// Multiplication matricielle carrée $N \times N$ optimisée pour le cache ($O(N^3)$).
///
/// Les matrices sont linéarisées par lignes (*row-major*). L'algorithme applique
/// l'ordre d'itération $i-k-j$ assurant un parcours séquentiel contigu de la matrice `b`.
///
/// # Arguments
///
/// * `a` - Matrice gauche linéarisée (taille $N^2$)
/// * `b` - Matrice droite linéarisée (taille $N^2$)
/// * `n` - Dimension de la matrice carrée ($N$)
///
/// # Erreurs
///
/// Renvoie [`MathError::ZeroDimension`] si `n == 0`.
/// Renvoie [`MathError::InvalidMatrixDimension`] si `a.len() != n * n` ou `b.len() != n * n`.
pub fn square_matrix_multiply(a: &[f64], b: &[f64], n: usize) -> Result<Vec<f64>, MathError> {
    if n == 0 {
        return Err(MathError::ZeroDimension);
    }

    let expected_len = n * n;
    if a.len() != expected_len {
        return Err(MathError::InvalidMatrixDimension {
            expected_len,
            actual_len: a.len(),
        });
    }
    if b.len() != expected_len {
        return Err(MathError::InvalidMatrixDimension {
            expected_len,
            actual_len: b.len(),
        });
    }

    let mut result = vec![0.0; expected_len];

    // Boucle i-k-j : maximise les hits dans le cache L1 de b
    for i in 0..n {
        let row_offset_a = i * n;
        let row_offset_res = i * n;
        for k in 0..n {
            let a_ik = a[row_offset_a + k];
            let row_offset_b = k * n;
            for j in 0..n {
                result[row_offset_res + j] += a_ik * b[row_offset_b + j];
            }
        }
    }

    Ok(result)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_dot_product_nominal() {
        let a = [1.0, 2.0, 3.0, 4.0];
        let b = [2.0, 0.5, -1.0, 3.0];
        // 1*2 + 2*0.5 + 3*(-1) + 4*3 = 2 + 1 - 3 + 12 = 12
        let res = dot_product(&a, &b).unwrap();
        assert!((res - 12.0).abs() < 1e-12);
    }

    #[test]
    fn test_dot_product_dimension_mismatch() {
        let a = [1.0, 2.0];
        let b = [1.0, 2.0, 3.0];
        assert!(matches!(
            dot_product(&a, &b),
            Err(MathError::DimensionMismatch {
                expected: 2,
                found: 3
            })
        ));
    }

    #[test]
    fn test_monte_carlo_pi_convergence() {
        let pi_approx = monte_carlo_pi(500_000, 1337);
        // Doit converger proche de 3.14159 avec une tolérance raisonnable
        assert!(
            (pi_approx - std::f64::consts::PI).abs() < 0.02,
            "Valeur obtenue : {pi_approx}"
        );
    }

    #[test]
    fn test_matrix_multiply_identity() {
        let n = 3;
        #[rustfmt::skip]
        let identity = [
            1.0, 0.0, 0.0,
            0.0, 1.0, 0.0,
            0.0, 0.0, 1.0,
        ];
        #[rustfmt::skip]
        let matrix = [
            1.0, 2.0, 3.0,
            4.0, 5.0, 6.0,
            7.0, 8.0, 9.0,
        ];

        let res = square_matrix_multiply(&matrix, &identity, n).unwrap();
        assert_eq!(res, matrix);
    }

    #[test]
    fn test_matrix_multiply_invalid_dimensions() {
        let invalid = [1.0, 2.0];
        assert!(matches!(
            square_matrix_multiply(&invalid, &invalid, 2),
            Err(MathError::InvalidMatrixDimension {
                expected_len: 4,
                actual_len: 2
            })
        ));
        assert!(matches!(
            square_matrix_multiply(&[], &[], 0),
            Err(MathError::ZeroDimension)
        ));
    }
}
