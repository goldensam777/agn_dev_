#!/usr/bin/env bash
set -euo pipefail

# scripts/verify.sh — Le Juge Souverain de la Forge
# Règle d'or : Ce script doit renvoyer 0 pour que toute tâche soit déclarée accomplie.

RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m'

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

echo -e "${BLUE}====================================================${NC}"
echo -e "${BLUE}          FORGE AGENTIQUE — CONTRÔLE DE QUALITÉ     ${NC}"
echo -e "${BLUE}====================================================${NC}"

FAILURES=0

# --- ÉTAPE 1 : Moteur Natif (C++ & AddressSanitizer) ---
echo -e "\n${YELLOW}[1/5] Vérification du Moteur Natif C++ (ASan & Bench)...${NC}"
if make -f native/Makefile clean > /dev/null 2>&1 && make -f native/Makefile; then
    echo -e "${GREEN}  ✓ C++ : 0 warning, 0 fuite mémoire, tests validés sous AddressSanitizer.${NC}"
else
    echo -e "${RED}  ✗ ÉCHEC : Le moteur C++ a des warnings ou des fuites mémoire.${NC}"
    FAILURES=$((FAILURES + 1))
fi

# --- ÉTAPE 2 : Frontière Contracts (Validation Zod & TypeScript) ---
echo -e "\n${YELLOW}[2/5] Vérification de la frontière contracts/ (Zod)...${NC}"
if npm run --workspace=contracts build > /dev/null 2>&1; then
    echo -e "${GREEN}  ✓ Contracts : Typage TypeScript strict et schémas Zod compilés.${NC}"
else
    echo -e "${RED}  ✗ ÉCHEC : Erreur de typage dans contracts/schemas.ts.${NC}"
    FAILURES=$((FAILURES + 1))
fi

# --- ÉTAPE 3 : Serveur & Pont Natif (Integration Tests) ---
echo -e "\n${YELLOW}[3/5] Vérification du Serveur & Bridge Natif (Node.js <-> C++)...${NC}"
if npm run --workspace=server test > /dev/null 2>&1 && npm run --workspace=server build > /dev/null 2>&1; then
    echo -e "${GREEN}  ✓ Serveur : Bridge natif et tests d'intégration validés.${NC}"
else
    echo -e "${RED}  ✗ ÉCHEC : Erreur dans le bridge natif ou les routes serveur.${NC}"
    FAILURES=$((FAILURES + 1))
fi

# --- ÉTAPE 4 : Interface Web (React + TS + Vite) ---
echo -e "\n${YELLOW}[4/5] Vérification du Frontend Web (React & TypeScript)...${NC}"
if npm run --workspace=web typecheck > /dev/null 2>&1 && npm run --workspace=web build > /dev/null 2>&1; then
    echo -e "${GREEN}  ✓ Web : Typage strict React et build Vite réussis.${NC}"
else
    echo -e "${RED}  ✗ ÉCHEC : Erreur de compilation dans l'interface web.${NC}"
    FAILURES=$((FAILURES + 1))
fi

# --- ÉTAPE 5 : Intégrité du Harness & Mémoire ---
echo -e "\n${YELLOW}[5/5] Vérification de l'intégrité du Harness...${NC}"
REQUIRED_FILES=(
    "README.md"
    "AGENTS.md"
    "CONVENTIONS.md"
    ".harness/knowledge/architecture.md"
    ".harness/knowledge/glossary.md"
    ".harness/knowledge/domain/guide_injection_domaine.md"
    ".harness/playbooks/add-native-module.md"
    ".harness/playbooks/add-endpoint.md"
    ".harness/examples/canonical_vector_core.cpp"
    ".harness/examples/canonical_pratt_parser_arena.cpp"
    ".harness/examples/canonical_kahan_summation.cpp"
    ".harness/examples/canonical_lexer_pratt.rs"
    ".harness/examples/canonical_arena_c23.c"
    ".harness/examples/canonical_branded_contracts.ts"
    ".harness/examples/canonical_scientific_canvas.tsx"
    ".harness/examples/canonical_worker_pipeline.js"
    ".harness/examples/canonical_vectorized_numpy.py"
    ".harness/knowledge/domain/fullstack/README.md"
    ".harness/knowledge/domain/scientific/README.md"
    ".github/workflows/ci.yml"
    "contracts/schemas.ts"
    "native/Makefile"
    "server/src/bridge/native_bridge.ts"
    "web/src/App.tsx"
    "web/src/pages/DashboardPage.tsx"
    "web/src/components/StatusBadge.tsx"
    ".forge/mcp/src/index.ts"
    ".harness/knowledge/languages/cpp/README.md"
    ".harness/knowledge/languages/rust/README.md"
    ".harness/knowledge/languages/c/README.md"
    ".harness/knowledge/languages/ts/README.md"
    ".harness/knowledge/languages/react/README.md"
    ".harness/knowledge/languages/js/README.md"
    ".harness/knowledge/languages/python/README.md"
    ".harness/knowledge/domain/compilers/README.md"
    ".harness/playbooks/init-language-corpus.md"
    "docs/references/README.md"
    "scripts/query_book.py"
)

HARNESS_OK=true
for f in "${REQUIRED_FILES[@]}"; do
    if [ ! -f "$f" ]; then
        echo -e "${RED}  ✗ Fichier requis manquant : $f${NC}"
        HARNESS_OK=false
        FAILURES=$((FAILURES + 1))
    fi
done

if [ "$HARNESS_OK" = true ]; then
    echo -e "${GREEN}  ✓ Harness complet : tous les guides, contrats et composants sont présents.${NC}"
fi

# --- VERDICT FINAL ---
echo -e "\n${BLUE}====================================================${NC}"
if [ $FAILURES -eq 0 ]; then
    echo -e "${GREEN}  VERDICT : PASS (Tous les critères de qualité sont satisfaits)${NC}"
    echo -e "${BLUE}====================================================${NC}"
    exit 0
else
    echo -e "${RED}  VERDICT : FAIL ($FAILURES échec(s) détecté(s))${NC}"
    echo -e "${BLUE}====================================================${NC}"
    exit 1
fi
