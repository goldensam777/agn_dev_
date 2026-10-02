# Playbook : Onboarding et Intervention sur un Projet Extérieur

Ce playbook définit la procédure standard pour déployer un agent d'IA sur un **projet externe** en s'appuyant sur le **Hub MCP Global** de la Forge.

---

## 1. Philosophie : Hub & Spoke

L'agent intervenant sur un projet tiers ne doit pas réinventer les règles d'architecture ni dupliquer inutilement la base de connaissances.
- **Le Hub Global (`agn_dev_`)** fournit les outils MCP (`forge_query_knowledge`, `forge_get_canonical_example`, `forge_audit_memory`, `forge_scaffold_harness`).
- **Le Projet Satellite** conserve son autonomie avec son propre `scripts/verify.sh`, son `AGENTS.md` léger et son journal de décisions d'architecture (`.harness/knowledge/decisions.md`).

---

## 2. Procédure Pas-à-Pas

### Étape 1 : Initialisation du Harnais Local

Dans le répertoire du projet extérieur, invoquer l'outil MCP :
```json
{
  "name": "forge_scaffold_harness",
  "arguments": {
    "targetDir": ".",
    "projectType": "rust", // ou "cpp", "c", "typescript", "python", "polyglot"
    "projectName": "mon-microservice"
  }
}
```
L'outil génère automatiquement :
1. `AGENTS.md` (court, pointant vers le Hub central de la Forge).
2. `CONVENTIONS.md` (invariants adaptés à la stack technique).
3. `scripts/verify.sh` (rendu exécutable `chmod +x`).
4. `.harness/knowledge/decisions.md` (journal ADR initialisé).

---

### Étape 2 : Consultation de la Base de Connaissances Experte

Pour toute question d'algorithmique avancée, de compilateurs, de gestion mémoire ou de typage strict, utiliser :
- `forge_query_knowledge` avec un mot-clé (ex: `"Pratt parser"`, `"SSA dominance"`, `"Arena allocation"`, `"Kahan summation"`).
- `forge_get_canonical_example` pour récupérer un code de référence complet et testé sous ASan :
  - `canonical_pratt_parser_arena.cpp` (C++)
  - `canonical_arena_c23.c` (C C23)
  - `canonical_lexer_pratt.rs` (Rust)
  - `canonical_branded_contracts.ts` (TypeScript)
  - `canonical_vectorized_numpy.py` (Python)

---

### Étape 3 : Audit Mémoire et Sanitizers

Pour tout module C/C++ écrit sur le projet externe :
- Appeler `forge_audit_memory` sur le fichier source.
- L'outil compile avec `-fsanitize=address,undefined -Wall -Wextra -Werror` et garantit 0 fuite mémoire avant toute intégration.

---

### Étape 4 : La Boucle de Rétroaction Souveraine

Avant de déclarer une tâche terminée sur le projet extérieur :
1. Lancer `forge_verify` (ou exécuter `bash scripts/verify.sh` dans le terminal).
2. Vérifier que le code de retour est strictement `0`.
3. Consigner tout choix architectural structurant dans `.harness/knowledge/decisions.md`.
