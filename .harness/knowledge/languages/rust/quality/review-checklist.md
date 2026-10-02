# Quality Rust : Grille de Revue & Recette Outillage

> Ce document fournit la grille de scoring binaire et la recette outillage multi-tier appliquée par le reviewer.

---

## 1. Grille d'Évaluation & Système de Scoring

Toute revue de code effectuée par l'agent ou un humain attribue un verdict basé sur trois niveaux de sévérité :

### Niveau 1 : Défauts Bloquants (Score 0 — Rejet Immédiat)
- Toute violation d'aliasing ou comportement indéterminé détecté par Miri.
- Présence d'un bloc `unsafe` sans commentaire `// SAFETY:` prouvant ses préconditions.
- Présence d'un `.unwrap()` ou d'un `.expect()` non justifié en production.
- Présence d'une fuite mémoire avérée ou d'un cycle fort de pointeurs.
- Blocage de l'exécuteur Tokio par une opération synchrone lourde.

### Niveau 2 : Défauts Majeurs (Pénalité -2 par occurrence)
- Appel à `.clone()` sur une structure non triviale à l'intérieur d'une boucle chaude.
- Paramètre de fonction acceptant `&String` ou `&Vec<T>` au lieu de `&str` ou `&[T]`.
- Utilisation de `std::collections::HashMap` standard dans un goulot d'étranglement de calcul.
- Allocation dynamique répétée sans `with_capacity`.

### Niveau 3 : Défauts Mineurs (Pénalité -1 par occurrence)
- Newtype omettant `#[repr(transparent)]`.
- Omission de `#[must_use]` sur une méthode de transformation retournant une nouvelle valeur.
- Manque de documentation Rustdoc sur un élément public d'une bibliothèque.

---

## 2. Règle Absolue de la Review Citante

Chaque commentaire de revue formulé par l'agent **doit obligatoirement citer le document de référence** du corpus Rust :
- Exemple : *"Rejet : Violation de `.harness/knowledge/languages/rust/core/ownership-borrowing.md §2` (le paramètre prend `&Vec<f64>` au lieu de `&[f64]`)."*
- Exemple : *"Rejet : Bloquant selon `.harness/knowledge/languages/rust/core/unsafe-miri.md §1` (bloc unsafe sans commentaire // SAFETY: explicite)."*

---

## 3. Recette Outillage Multi-Tier (Pour scripts/verify.sh)

Pour valider un module Rust, le pipeline d'outillage doit exécuter la chaîne suivante :

```bash
# Tier 1 : Compilation stricte
cargo check --all-targets

# Tier 2 : Linter expert Pedantic (Zéro avertissement toléré)
cargo clippy --all-targets -- -D clippy::all -D clippy::pedantic

# Tier 3 : Tests unitaires et d'intégration
cargo test

# Tier 4 : Vérification formelle du code unsafe sous Miri (si unsafe présent)
MIRIFLAGS="-Zmiri-tree-borrows" cargo miri test

# Tier 5 : Bancs de mesure de performance
cargo bench
```
