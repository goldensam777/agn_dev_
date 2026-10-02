# AGENTS.md — Manifeste de la Forge Agentique & Guide de Continuité

Ce document est la **mémoire centrale** et le **fil conducteur** du projet. Tout agent d'IA intervenant sur ce dépôt doit impérativement lire et respecter ce document ainsi que [CONVENTIONS.md](file:///home/samuelyevi/dev/leumas-experience/agn_dev_/CONVENTIONS.md).

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
3. **Palier 3 : Ingénierie de Langages de Programmation (Le sommet)**
   - Conception et implémentation de nouveaux langages de programmation (notamment le projet **Onyx**).
   - Génération de code optimisé via l'infrastructure **LLVM**.
   - Bancs de mesure et comparatifs de performance extrêmes (le rôle de **Mercuria**).

---

## 2. Architecture Cible du Dépôt

Le projet applique le modèle **Agent Harness & Strict Boundaries** :

```
agn_dev_/
├── AGENTS.md                 # Mémoire persistante du projet et guide pour les agents (ce fichier)
├── CONVENTIONS.md            # Règles absolues de style, commandes de test, invariants
├── package.json              # Monorepo / Workspaces (web, server, contracts)
├── .forge/                   # Infrastructure de sandbox
│   ├── Dockerfile            # Environnement complet isolé (compilateurs, ASan, Valgrind, LLVM)
│   └── docker-compose.yml    # Configuration d'exécution sécurisée
├── .harness/                 # Base de connaissances et modes opératoires pour l'agent
│   ├── knowledge/            # CE QU'IL DOIT SAVOIR
│   │   ├── architecture.md   # Carte du projet et décisions prises (ADR)
│   │   ├── domain/           # Savoir scientifique, spécifications d'Onyx...
│   │   └── glossary.md       # Vocabulaire et définitions du projet
│   ├── playbooks/            # COMMENT RÉALISER UNE TÂCHE
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
│   └── bench/                # Bancs de mesure de performance (Mercuria)
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
- [ ] **Étape 1 : Fondation du Harness**
  - [x] Création de `AGENTS.md`.
  - [ ] Création de `CONVENTIONS.md`.
  - [ ] Mise en place de l'arborescence `.harness/` (glossaire, architecture, premier playbook).
- [ ] **Étape 2 : Le Juge & la Sandbox**
  - [ ] Écriture de `scripts/verify.sh`.
  - [ ] Configuration de la sandbox `.forge/Dockerfile`.
- [ ] **Étape 3 : Frontière `contracts/` & Premier Module `native/`**
  - [ ] Initialisation de `contracts/` avec un premier schéma Zod.
  - [ ] Création d'un module C++ ou Rust avec benchmark et tests sanitizers.
- [ ] **Étape 4 : Orchestration Fullstack & Interface Scientifique**
  - [ ] Liaison `server/` (bridge natif) et `web/` (React).
