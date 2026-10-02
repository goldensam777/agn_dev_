# Quality C : Grille de Revue & Recette Outillage Multi-Tier

> Ce document fournit la grille de scoring binaire et la recette outillage appliquée par le reviewer pour le langage C.

---

## 1. Grille d'Évaluation & Système de Scoring

Toute revue de code en C attribue un verdict basé sur trois niveaux de sévérité :

### Niveau 1 : Défauts Bloquants (Score 0 — Rejet Immédiat)
- Toute détection de fuite mémoire ou dépassement de tampon sous `AddressSanitizer` ou `Valgrind`.
- Tout comportement indéterminé signalé par `UBSan` (overflow signé, shift invalide, pointeur non aligné).
- Présence de fonctions vulnérables (`gets`, `strcpy`, `strcat`, `sprintf`).
- Absence de libération sur un chemin de code d'erreur (non-respect de `goto cleanup`).
- Appel non async-signal-safe dans un gestionnaire de signal.

### Niveau 2 : Défauts Majeurs (Pénalité -2 par occurrence)
- Absence de vérification du retour de `malloc` / `mmap`.
- Absence d'affectation `ptr = nullptr` immédiatement après `free(ptr)`.
- Absence du mot-clé `restrict` sur les buffers de calcul numérique contigus.
- Omission de `O_CLOEXEC` sur un descripteur de fichier.

### Niveau 3 : Défauts Mineurs (Pénalité -1 par occurrence)
- Usage de `NULL` au lieu de `nullptr` (C23).
- Omission de `[[nodiscard]]` sur une fonction retournant un code de statut.
- Utilisation de `#define` au lieu de `constexpr` pour une constante mathématique.

---

## 2. Règle de la Review Citante

Chaque rejet de code doit citer formellement le document et paragraphe correspondant :
- Exemple : *"Rejet : Bloquant selon `.harness/knowledge/languages/c/core/error-handling.md §1` (fuite mémoire du buffer sur le chemin de retour d'erreur ligne 45, pattern goto cleanup non appliqué)."*

---

## 3. Recette Outillage Multi-Tier (Pour scripts/verify.sh)

```bash
# Tier 1 : Compilation stricte C23 (Zéro warning toléré)
clang -std=c23 -Wall -Wextra -Wpedantic -Wconversion -Werror -c mon_module.c -o /dev/null

# Tier 2 : Analyse sous Sanitizers (ASan + UBSan)
clang -std=c23 -Wall -Wextra -Werror -fsanitize=address,undefined -g test_module.c -o test_bin && ./test_bin

# Tier 3 : Analyse statique approfondie
clang-tidy mon_module.c -- -std=c23
```
