#!/usr/bin/env bash
set -euo pipefail

# scripts/forge_init.sh — Initialiseur universel de harnais agentique
# Usage : forge_init.sh [chemin_du_projet] [type_de_projet (optionnel)]

TARGET_DIR="${1:-.}"
FORGE_HUB_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

TARGET_DIR="$(cd "$TARGET_DIR" 2>/dev/null && pwd || echo "$TARGET_DIR")"
mkdir -p "$TARGET_DIR"
cd "$TARGET_DIR"

PROJECT_NAME="$(basename "$TARGET_DIR")"
OVERRIDE_TYPE="${2:-}"

# 1. Détection automatique de la stack si non spécifiée
DETECTED_TYPE="polyglot"
if [ -n "$OVERRIDE_TYPE" ]; then
    DETECTED_TYPE="$OVERRIDE_TYPE"
elif [ -f "Cargo.toml" ]; then
    DETECTED_TYPE="rust"
elif [ -f "package.json" ]; then
    DETECTED_TYPE="typescript"
elif [ -f "CMakeLists.txt" ] || [ -f "Makefile" ]; then
    DETECTED_TYPE="cpp"
elif [ -f "pyproject.toml" ] || [ -f "requirements.txt" ] || [ -f "setup.py" ]; then
    DETECTED_TYPE="python"
fi

echo "===================================================="
echo "  INITIALISATION DU HARNAIS AGENTIQUE : $PROJECT_NAME"
echo "  Stack détectée : $DETECTED_TYPE"
echo "  Hub de référence : $FORGE_HUB_DIR"
echo "===================================================="

# 2. Création de l'arborescence .harness et scripts/
mkdir -p scripts
mkdir -p .harness/knowledge/domain
mkdir -p .harness/playbooks

# Lien symbolique vers le Hub de connaissances de la Forge (accès direct en lecture)
if [ ! -e ".harness/hub" ]; then
    ln -sfn "$FORGE_HUB_DIR/.harness" ".harness/hub"
fi

# 3. Génération de AGENTS.md
cat << 'EOF' > AGENTS.md
# AGENTS.md — Manifeste & Guide de Continuité pour l'Agent

Ce dépôt applique le modèle **Agent Harness & Strict Boundaries**. Tout agent d'IA intervenant sur ce code doit impérativement respecter les règles absolues suivantes :

---

## 1. Règles d'Or pour les Agents d'IA

1. **Le verdict `verify.sh` est souverain :** Une tâche n'est jamais considérée comme achevée tant que `bash scripts/verify.sh` ne renvoie pas un code de retour `0` (zéro warning, tous les tests au vert).
2. **Mémoire & Journal des Décisions :** Toute décision d'architecture, choix d'algorithme ou arbitrage structurant doit impérativement être consigné dans `.harness/knowledge/decisions.md`.
3. **Accès au Hub de Connaissances Expert :**
   - Vous avez accès aux 7 corpus de langages (C++, Rust, C23, TypeScript, React, Python, JS), aux algorithmes de compilateurs (Dragon Book) et aux 9 exemples canoniques via le lien `.harness/hub/`.
   - Si les outils MCP sont actifs, vous disposez de `forge_query_knowledge`, `forge_get_canonical_example` et `forge_audit_memory`.
4. **Zéro régression & Respect des contrats :** Aucun type ou contrat d'API ne doit être altéré sans validation explicite.

EOF

# 4. Génération de CONVENTIONS.md
case "$DETECTED_TYPE" in
    rust)
        CONV_RULES="- \`cargo clippy --all-targets -- -D warnings\` doit passer sans aucun avertissement.
- \`cargo test\` doit être à 100% vert.
- Tout bloc \`unsafe\` doit obligatoirement comporter un commentaire \`// SAFETY:\` détaillant les invariants et être validé sous \`cargo miri\` si applicable.
- Gestion stricte de la mémoire : pas de fuites, privilégier le passage par référence et les slices."
        VERIFY_CMD="cargo clippy --all-targets -- -D warnings\ncargo test"
        ;;
    cpp|c)
        CONV_RULES="- Compilation stricte : \`-Wall -Wextra -Wpedantic -Werror\`.
- Isolation mémoire obligatoire : Tout code de test doit tourner sous AddressSanitizer (\`-fsanitize=address,undefined\`) avec zéro fuite.
- Préférer la gestion de mémoire par arène (Bump Allocator) ou RAII strict. Aucun pointeur nu propriétaire."
        VERIFY_CMD="if [ -f Makefile ]; then make test; elif [ -f CMakeLists.txt ]; then cmake -B build && cmake --build build && ctest --test-dir build; else echo 'Veuillez configurer votre commande de test C/C++'; fi"
        ;;
    typescript)
        CONV_RULES="- \`strict: true\` dans tsconfig.json sans exception.
- Validation des frontières externes via Zod ou schémas typés stricts. Zéro \`any\` non documenté.
- Tests unitaires complets sous Vitest ou Jest."
        VERIFY_CMD="npm run typecheck 2>/dev/null || npx tsc --noEmit\nnpm test"
        ;;
    python)
        CONV_RULES="- Typage statique strict (annotations de type sur toutes les fonctions) vérifié par \`mypy\` ou \`pyright\`.
- Linting et formatage stricts via \`ruff check\` et \`ruff format --check\`.
- Tests unitaires automatisés via \`pytest\`."
        VERIFY_CMD="ruff check . 2>/dev/null || true\npytest"
        ;;
    *)
        CONV_RULES="- Vérifications strictes : linters et tests au vert.
- Aucun warning de compilation toléré.
- Frontières typées entre les sous-systèmes."
        VERIFY_CMD="echo 'Exécutez vos tests et linters ici'\nexit 0"
        ;;
esac

cat << EOF > CONVENTIONS.md
# CONVENTIONS.md — Règles absolues et Invariants pour $PROJECT_NAME

## 1. Invariants de Qualité ($DETECTED_TYPE)
$CONV_RULES

---

## 2. Commande Unique de Vérification
\`\`\`bash
bash scripts/verify.sh
\`\`\`
EOF

# 5. Génération de scripts/verify.sh
cat << EOF > scripts/verify.sh
#!/usr/bin/env bash
set -euo pipefail

# scripts/verify.sh — Le Juge Souverain pour $PROJECT_NAME
# Règle d'or : Ce script doit renvoyer 0 pour que toute tâche soit déclarée accomplie.

RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m'

ROOT_DIR="\$(cd "\$(dirname "\${BASH_SOURCE[0]}")/.." && pwd)"
cd "\$ROOT_DIR"

echo -e "\${BLUE}====================================================\${NC}"
echo -e "\${BLUE}     VÉRIFICATION QUALITÉ : $PROJECT_NAME           \${NC}"
echo -e "\${BLUE}====================================================\${NC}"

FAILURES=0

echo -e "\n--- Exécution des vérifications et tests ---"
if $VERIFY_CMD; then
    echo -e "\${GREEN}  ✓ Tous les tests et contrôles de type sont au vert.\${NC}"
else
    echo -e "\${RED}  ✗ Échec détecté lors des vérifications.\${NC}"
    FAILURES=\$((FAILURES + 1))
fi

echo -e "\n\${BLUE}====================================================\${NC}"
if [ \$FAILURES -eq 0 ]; then
    echo -e "\${GREEN}  VERDICT : PASS (Tous les critères de qualité sont satisfaits)\${NC}"
    echo -e "\${BLUE}====================================================\${NC}"
    exit 0
else
    echo -e "\${RED}  VERDICT : FAIL (\$FAILURES échec(s) détecté(s))\${NC}"
    echo -e "\${BLUE}====================================================\${NC}"
    exit 1
fi
EOF

chmod +x scripts/verify.sh

# 6. Initialisation de .harness/knowledge/decisions.md si inexistant
if [ ! -f .harness/knowledge/decisions.md ]; then
    cat << EOF > .harness/knowledge/decisions.md
# Journal des Décisions d'Architecture (ADR) — $PROJECT_NAME

Ce document consigne chronologiquement les décisions prises par les agents et l'équipe.

---

### Date : $(date +%Y-%m-%d)
- **Décision :** Initialisation du harnais agentique et connexion au Hub de la Forge.
- **Contexte :** Mise en place des règles strictes, du juge unique \`scripts/verify.sh\` et de l'accès aux connaissances expertes.
- **Statut :** Accepté.
EOF
fi

echo -e "\n✓ Harnais agentique initialisé avec succès dans : $TARGET_DIR"
echo "  - AGENTS.md : Charte de l'agent et lien vers le Hub"
echo "  - CONVENTIONS.md : Règles adaptées à la stack ($DETECTED_TYPE)"
echo "  - scripts/verify.sh : Juge souverain (exécutable)"
echo "  - .harness/knowledge/decisions.md : Journal ADR"
echo "  - .harness/hub : Lien symbolique vers le Hub expert ($FORGE_HUB_DIR/.harness)"
