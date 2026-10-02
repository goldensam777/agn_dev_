# Quality TS : Grille de Revue & Recette Outillage Multi-Tier

> Ce document fournit la grille d'évaluation et la recette outillage appliquée pour TypeScript.

---

## 1. Grille d'Évaluation & Système de Scoring

### Niveau 1 : Défauts Bloquants (Score 0 — Rejet Immédiat)
- Présence de `any` ou d'un cast `as Type` non sécurisé par un type guard ou un schéma Zod.
- Promesse non capturée (*unhandled promise / floating promise*).
- Absence de branche d'exhaustivité (`assertNever`) dans un switch sur union discriminée.
- Opération CPU bloquante exécutée sur le thread principal Node.js.

### Niveau 2 : Défauts Majeurs (Pénalité -2 par occurrence)
- Absence de Branded Types pour des identifiants métier.
- Assertion non-nulle `!` non prouvée par une vérification immédiate antérieure.
- Mutation en place d'un objet d'état partagé.
- Enums TypeScript numériques classiques au lieu d'unions de chaînes.

### Niveau 3 : Défauts Mineurs (Pénalité -1 par occurrence)
- Usage de `type` au lieu de `interface` pour un contrat d'objet standard.
- Usage de `delete` sur un objet altérant les classes cachées V8.
- Absence d'export nommé au profit d'un `export default`.

---

## 2. Règle de la Review Citante

Chaque commentaire de rejet de code doit citer formellement le document et paragraphe correspondant :
- Exemple : *"Rejet : Bloquant selon `.harness/knowledge/languages/ts/core/contracts-validation.md §1` (donnée reçue via fetch castée avec 'as UserProfile' sans passer par un schéma Zod)."*

---

## 3. Recette Outillage Multi-Tier (Pour scripts/verify.sh)

```bash
# Tier 1 : Vérification stricte des types sans émission
npx tsc --noEmit

# Tier 2 : Tests unitaires et d'intégration
npm test

# Tier 3 : Linter avec règles de typage strictes
npx eslint . --max-warnings 0
```
