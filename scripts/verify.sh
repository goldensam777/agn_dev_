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

LOG_DIR="${ROOT_DIR}/build/verify_logs"
mkdir -p "$LOG_DIR"

echo -e "${BLUE}====================================================${NC}"
echo -e "${BLUE}          FORGE AGENTIQUE — CONTRÔLE DE QUALITÉ     ${NC}"
echo -e "${BLUE}====================================================${NC}"

FAILURES=0

run_step() {
    local step_tag="$1"
    local step_title="$2"
    local log_file="$3"
    shift 3

    echo -e "\n${YELLOW}${step_tag} ${step_title}...${NC}"
    mkdir -p "$(dirname "$log_file")"
    if "$@" > "$log_file" 2>&1; then
        echo -e "${GREEN}  ✓ Succès : ${step_title}.${NC}"
    else
        echo -e "${RED}  ✗ ÉCHEC : ${step_title} (Consultez ${log_file})${NC}"
        echo -e "${RED}--- 30 dernières lignes de $(basename "$log_file") ---${NC}"
        tail -n 30 "$log_file" || true
        echo -e "${RED}--- Fin du journal d'erreur ---${NC}"
        FAILURES=$((FAILURES + 1))
    fi
}

# --- ÉTAPE 1 : Moteur Natif C++ (ASan & Moteur Onyx) ---
step1_native() {
    make -f native/Makefile clean
    make -f native/Makefile
}
run_step "[1/6]" "Vérification du Moteur Natif C++ (ASan & Moteur Onyx)" "$LOG_DIR/step1_native.log" step1_native

# --- ÉTAPE 2 : Non-Régression du Banc de Mesure (Médiane vs Baseline 5%) ---
step2_bench() {
    python3 scripts/compare_bench.py --baseline native/bench/baseline.json --bin bin/bench_math_core --runs 3 --threshold 5.0
}
run_step "[2/6]" "Comparaison du Banc à baseline.json (Médiane de 3 runs, seuil 5%)" "$LOG_DIR/step2_bench.log" step2_bench

# --- ÉTAPE 3 : Frontière Contracts (Validation Zod & TypeScript) ---
step3_contracts() {
    npm run --workspace=contracts build
}
run_step "[3/6]" "Vérification de la frontière contracts/ (Zod & TypeScript)" "$LOG_DIR/step3_contracts.log" step3_contracts

# --- ÉTAPE 4 : Serveur & Pont Natif (Integration Tests) ---
step4_server() {
    npm run --workspace=server test
    npm run --workspace=server build
}
run_step "[4/6]" "Vérification du Serveur & Bridge Natif (Node.js <-> C++)" "$LOG_DIR/step4_server.log" step4_server

# --- ÉTAPE 5 : Interface Web (React + TS + Vite) ---
step5_web() {
    npm run --workspace=web typecheck
    npm run --workspace=web build
}
run_step "[5/6]" "Vérification du Frontend Web (React & TypeScript)" "$LOG_DIR/step5_web.log" step5_web

# --- ÉTAPE 6 : Intégrité du Harness & Mémoire ---
REQUIRED_FILES=(
    "README.md"
    "AGENTS.md"
    "CONVENTIONS.md"
    ".harness/knowledge/architecture.md"
    ".harness/knowledge/glossary.md"
    ".harness/knowledge/domain/guide_injection_domaine.md"
    ".harness/playbooks/add-native-module.md"
    ".harness/playbooks/add-endpoint.md"
    ".harness/playbooks/init-language-corpus.md"
    ".harness/playbooks/onboard-external-repo.md"
    ".harness/playbooks/add-language-from-spec.md"
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
    "native/bench/baseline.json"
    "scripts/compare_bench.py"
    "scripts/forge_init.sh"
    "scripts/query_book.py"
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
    ".harness/knowledge/domain/DOCUMENTATIONS.md"
    "examples/onyx/arena.hpp"
    "examples/onyx/token.hpp"
    "examples/onyx/lexer.hpp"
    "examples/onyx/ast.hpp"
    "examples/onyx/parser.hpp"
    "examples/onyx/linear_check.hpp"
    "examples/onyx/value.hpp"
    "examples/onyx/runtime.hpp"
    "examples/test_onyx.cpp"
    "docs/references/README.md"
)

step6_harness() {
    local harness_ok=true
    for f in "${REQUIRED_FILES[@]}"; do
        if [ ! -f "$f" ]; then
            echo "  ✗ Fichier requis manquant : $f"
            harness_ok=false
        fi
    done
    if [ "$harness_ok" = false ]; then
        return 1
    fi
    echo "  ✓ Tous les fichiers requis du Harness sont présents."
    return 0
}
run_step "[6/6]" "Vérification de l'intégrité du Harness et Mémoire" "$LOG_DIR/step6_harness.log" step6_harness

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
