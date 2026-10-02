# AGENTS.md — Manifeste de la Forge Agentique & Guide de Continuité

Ce document est la **mémoire centrale** et le **fil conducteur** du projet. Tout agent d'IA intervenant sur ce dépôt doit impérativement lire et respecter ce document ainsi que [CONVENTIONS.md](CONVENTIONS.md).

---

## 1. Vision et Objectifs Suprêmes

L'objectif de cet environnement n'est pas de produire des prototypes jetables, mais de bâtir une **forge logicielle autonome de haute précision**, capable de concevoir du code de **qualité industrielle irréprochable** où chaque aspect est pris en compte :
- **Gestion fine de la mémoire** (zéro fuite, allocations maîtrisées, cache-friendly).
- **Architecture & Design logiciel** (séparation stricte des responsabilités, Clean Architecture, contrats immuables).
- **Rigueur algorithmique** (complexité optimale, micro-benchmarks vérifiés, pas de régressions de performance).
- **Typage et sémantique stricts** (aucun warning de compilation, typage bout-en-bout).

### Langages au périmètre
- **Système & Haute Performance :** C, C++, Rust
- **Fullstack & Interface :** TypeScript, React, JavaScript, Node.js
- **Scientifique & Données :** Python

### Paliers d'évolution du projet
1. **Palier 1 : La Forge & la Sandbox**
   - Mise en place de l'infrastructure d'isolation (Docker, sanitizers, linters).
   - Boucle de rétroaction fermée via le juge unique `scripts/verify.sh`.
2. **Palier 2 : Applications Fullstack Scientifiques**
   - Cœur de calcul numérique haute performance en C++ / Rust.
   - Pont de communication typé et orchestrateur Node.js / TypeScript.
   - Interface frontend React / TypeScript avec rendu visuel haute fidélité (WebGL / WebGPU).
3. **Palier 3 : Ingénierie de Langages de Programmation (Compilateurs & Runtimes)**
   - Conception et implémentation de nouveaux langages de programmation (compilateurs, interpréteurs, DSL).
   - Génération de code optimisé via l'infrastructure **LLVM**.
   - Bancs de mesure et comparatifs de performance extrêmes (profilage CPU/mémoire/cache L1/L2).

---

## 2. Architecture Cible du Dépôt

Le projet applique le modèle **Agent Harness & Strict Boundaries** :

```
agn_dev_/
├── AGENTS.md                 # Mémoire persistante du projet et guide pour les agents (ce fichier)
├── CONVENTIONS.md            # Règles absolues de style, commandes de test, invariants
├── package.json              # Monorepo / Workspaces (web, server, contracts)
├── .forge/                   # Infrastructure de sandbox & outils agents
│   ├── Dockerfile            # Environnement complet isolé (compilateurs, ASan, Valgrind, LLVM)
│   ├── docker-compose.yml    # Configuration d'exécution sécurisée
│   └── mcp/                  # Surcouche MCP personnalisée (outils composites pour projets denses)
│       ├── package.json      # SDK officiel @modelcontextprotocol/sdk
│       └── src/              # Outils chirurgicaux (audit mémoire, profilage, analyseurs AST)
├── .harness/                 # Base de connaissances et modes opératoires pour l'agent
│   ├── knowledge/            # CE QU'IL DOIT SAVOIR
│   │   ├── architecture.md   # Carte du projet et décisions prises (ADR)
│   │   ├── domain/           # Savoir scientifique, spécifications de langages / algorithmes
│   │   ├── languages/        # Corpus experts par langage (ex: cpp/ avec core, domains, quality)
│   │   └── glossary.md       # Vocabulaire et définitions du projet
│   ├── playbooks/            # COMMENT RÉALISER UNE TÂCHE
│   │   ├── init-language-corpus.md # Initialisation et maintien d'un corpus de connaissances
│   │   ├── add-endpoint.md   # Procédure pas-à-pas pour une nouvelle route
│   │   ├── add-component.md  # Procédure pour un composant React
│   │   └── add-native-module.md # Procédure pour ajouter un module C++ ou Rust
│   └── examples/             # Code de référence validé et modèles à imiter
├── scripts/
│   └── verify.sh             # LE JUGE : lance toutes les vérifications, verdict unique PASS/FAIL
├── contracts/                # LA FRONTIÈRE IMMUABLE
│   ├── schemas.ts            # Schémas de données et types partagés (Zod)
│   └── messages.md           # Spécification des protocoles de communication et erreurs
├── web/                      # Interface utilisateur (React + TypeScript)
│   ├── src/
│   │   ├── components/
│   │   ├── pages/
│   │   └── api/              # Appels typés via contracts/
│   └── tests/                # Tests unitaires et d'interface (Vitest)
├── server/                   # Orchestrateur backend (Node.js + TypeScript)
│   ├── src/
│   │   ├── routes/
│   │   └── bridge/           # Seul et unique point d'accès au code natif
│   └── tests/
├── native/                   # Calcul lourd & systèmes (C++ / Rust)
│   ├── CMakeLists.txt / Cargo.toml
│   ├── include/
│   ├── src/
│   ├── tests/                # Tests unitaires (GoogleTest / Catch2 / cargo test)
│   └── bench/                # Bancs de mesure de performance et profilage
└── .github/workflows/        # Intégration continue miroir de scripts/verify.sh
```

---

## 3. Règles d'Or pour les Agents d'IA

1. **Le verdict `verify.sh` est souverain :** Une tâche n'est jamais considérée comme achevée tant que `bash scripts/verify.sh` ne renvoie pas un code de retour `0` (zéro warning, zéro fuite mémoire, tous les tests au vert).
2. **Inviolabilité de la frontière `contracts/` :** Aucune modification de contrat d'API ne doit être faite sans mettre à jour simultanément les validateurs Zod et la documentation dans `contracts/`.
3. **Respect de l'isolation mémoire :** Tout code C/C++ doit être compilé avec `-fsanitize=address,undefined` et validé sans la moindre erreur. Tout code Rust `unsafe` doit être justifié et passé sous `miri`.
4. **Adoption des Playbooks :** Avant de réaliser une modification majeure, l'agent doit consulter le playbook correspondant dans `.harness/playbooks/`.
5. **Mise à jour continue de ce fichier :** Tout changement architectural d'envergure doit être consigné dans ce document.

---

## 4. État d'Avancement et Feuille de Route

- [x] **Étape 0 : Audit de l'environnement hôte** *(Docker 29.7, Clang 22, GCC 16, Rust 1.97, Node 24, Python 3.14 vérifiés)*.
- [x] **Étape 1 : Fondation du Harness**
  - [x] Création de `AGENTS.md` et `CONVENTIONS.md`.
  - [x] Base de connaissances `.harness/` (architecture, glossaire, playbooks).
- [x] **Étape 2 : Le Juge & la Sandbox**
  - [x] Écriture de `scripts/verify.sh` (verdict unique automatisé).
  - [x] Sandbox `.forge/Dockerfile` & `docker-compose.yml`.
  - [x] Surcouche MCP composite pour projets denses (`.forge/mcp/`).
- [x] **Étape 3 : Frontière `contracts/` & Premier Module `native/`**
  - [x] Initialisation de `contracts/` avec schémas Zod (santé, calcul scientifique, benchmarks).
  - [x] Moteur de calcul C++ dans `native/` validé sous AddressSanitizer (0 fuite) & banc de mesure (716 Mops/s).
- [x] **Étape 4 : Orchestration Fullstack & Interface Scientifique**
  - [x] Liaison `server/` (bridge natif vers Node.js / TypeScript avec tests d'intégration).
  - [x] Application `web/` (React 19 + TypeScript + tableau de bord scientifique temps réel).
- [x] **Étape 4.5 : Bibliothèque de Connaissances Multi-Langages Haute Précision**
  - [x] Structure standardisée (index `README.md`, 5 fondamentaux `core/`, 3 spécialités `domains/`, 2 contrôles `quality/`).
  - [x] 7 corpus experts exhaustifs (77 documents de référence) : C++, Rust, C (C23), TypeScript, React 19, JavaScript, Python (3.12/3.13/3.14).
  - [x] Règle de citation obligatoire dans les revues et intégration dans le juge souverain `scripts/verify.sh`.
- [x] **Étape 4.8 : Ingestion Théorique des Compilateurs (Le « Corpus Dragon »)**
  - [x] Bibliothèque de références `docs/references/` (*Dragon Book* 2e éd., *Crafting Interpreters*).
  - [x] Outil d'interrogation chirurgicale rapide `scripts/query_book.py`.
  - [x] 8 synthèses algorithmiques fondamentales dans `.harness/knowledge/domain/compilers/` (Automates, Pratt/LR, SDD/AST, SSA/Dominance, Run-time/GC, CFG/Tree tiling, Dataflow equations, Chaitin-Briggs).
- [x] **Étape 4.9 : Exemples Canoniques & Connaissances Transverses de Production**
  - [x] 9 exemples canoniques complets dans `.harness/examples/` (C++ Pratt & Kahan, Rust Pratt, C23 Arena, TS Branded, React 19 WebGL Canvas, JS Worker Pipeline, Python Vectorized NumPy).
  - [x] Corpus transverse dans `.harness/knowledge/domain/fullstack/` (mesure, KPIs, Web Vitals, p99, architecture stateless, observabilité RED/USE, conteneurs durcis).
  - [x] Corpus transverse dans `.harness/knowledge/domain/scientific/` (reproductibilité déterministe, sécurité frontière NaN/Inf, tolérances, orchestration asynchrone).
- [ ] **Étape 5 : Ingénierie de Langages (Compilateurs, Interpréteurs & LLVM)**
  - [x] Spécification formelle du langage système **Onyx** (`.harness/knowledge/domain/DOCUMENTATIONS.md`) : syntaxe par indentation, typage linéaire zéro-GC, promotion numérique, algèbre des quaternions et tenseurs multidimensionnels contigus.
  - [x] Moteur et compilateur natif C++20 sans Garbage Collector (`native/onyx/`) :
    - Allocateur d'arène contigu (`Arena`) avec suivi déterministe des destructeurs non-triviaux (zéro fuite sous ASan).
    - Lexer à pile d'indentation indent/dedent et commentaires `(< ... >)`.
    - Parseur Pratt 10 niveaux de priorité avec associativité à droite de la puissance `^`.
    - Analyseur statique de consommation linéaire compile-time (`LinearChecker` détectant le use-after-consume sans GC).
    - Runtime d'exécution sans GC supportant nombres complexes, quaternions non-commutatifs et tenseurs contigus.
    - 8 bancs d'essai complets (`native/tests/test_onyx.cpp`) intégrés à `native/Makefile` et `scripts/verify.sh` sous ASan.
  - [ ] Génération de code intermédiaire LLVM IR.
  - [ ] Bancs de mesure comparatifs multi-langages.
