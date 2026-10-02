# ⚡ Forge Agentique & Laboratoire de R&D

> Environnement autonome et bac à sable de haute précision pour le développement agentique, le calcul scientifique et l'ingénierie de langages de programmation.

---

## 🎯 Vision & Principes Fondamentaux

Cette forge n'est pas conçue pour des prototypes jetables, mais comme une **usine logicielle de niveau industriel** où chaque ligne de code est contrôlée :
- **Zéro fuite mémoire** : Tout code C/C++ est systématiquement validé sous `AddressSanitizer` (ASan) et `UBSan`.
- **Typage strict & frontières scellées** : Les échanges entre le web, le serveur et les moteurs natifs sont strictement validés par des contrats **Zod** (`contracts/`).
- **Juge unique et souverain** : Une tâche n'est déclarée terminée que si `bash scripts/verify.sh` retourne le code de sortie `0`.
- **Isolation sécurisée** : Exécution hermétique via conteneur Docker (`.forge/`) et outillage composite MCP (`@modelcontextprotocol/sdk`).

---

## 🏛️ Arborescence du Dépôt

```
agn_dev_/
├── README.md                 # Entrée principale pour les humains et les agents
├── AGENTS.md                 # Mémoire persistante, vision et feuille de route
├── CONVENTIONS.md            # Règles absolues de style, compilation et mémoire
├── package.json              # Monorepo Workspaces (contracts, server, web, mcp)
├── .forge/                   # Infrastructure de sandbox & outils agents
│   ├── Dockerfile            # Bac à sable isolé (compilateurs, ASan, Valgrind, LLVM)
│   ├── docker-compose.yml    # Configuration d'exécution sécurisée avec limites CPU/RAM
│   └── mcp/                  # Serveur MCP officiel (outils chirurgicaux pour agents)
├── .harness/                 # Cerveau de l'agent (modes opératoires et savoir)
│   ├── knowledge/            # Architecture (ADR), glossaire, injection de domaine, languages/ (C++, Rust...)
│   ├── playbooks/            # Guides pas-à-pas (init-language-corpus, add-endpoint, add-native-module...)
│   └── examples/             # Code canonique et modèles d'implémentation
├── scripts/
│   └── verify.sh             # LE JUGE : lance tous les vérificateurs, verdict PASS/FAIL
├── contracts/                # LA FRONTIÈRE IMMUABLE
│   ├── schemas.ts            # Schémas de données et types partagés (Zod)
│   └── messages.md           # Spécification des protocoles de communication et erreurs
├── native/                   # Cœur de calcul & systèmes (C++ / Rust)
│   ├── include/ & src/       # Algorithmes SIMD et compilateurs
│   ├── tests/                # Tests unitaires validés sous AddressSanitizer
│   └── bench/                # Bancs de mesure de performance réels
├── server/                   # Orchestrateur backend (Node.js + TypeScript)
│   ├── src/bridge/           # Seul et unique pont sécurisé vers le binaire C++
│   └── src/routes/           # API REST typée
├── web/                      # Interface utilisateur (React 19 + TypeScript)
│   ├── src/components/       # Composants graphiques modulaires
│   ├── src/pages/            # Tableaux de bord de contrôle scientifique
│   └── src/api/              # Client d'appel typé via contracts/
└── .github/workflows/        # CI GitHub Actions (miroir de scripts/verify.sh)
```

---

## 🚀 Démarrage Rapide

### 1. Lancer le Juge de Vérification
Pour tester l'ensemble du monorepo (moteur natif, mémoire ASan, contrats Zod, bridge et interface web) :
```bash
bash scripts/verify.sh
```

### 2. Lancer l'Application en Développement
```bash
# Démarrer le serveur API (port 3001)
npm run --workspace=server dev

# Démarrer l'interface React (port 5173)
npm run --workspace=web dev
```

---

## 📖 Documentation Agentique

- Pour comprendre les règles et invariants : consulter [CONVENTIONS.md](CONVENTIONS.md).
- Pour l'historique et la feuille de route : consulter [AGENTS.md](AGENTS.md).
- Pour injecter un nouveau langage ou projet scientifique : consulter [.harness/knowledge/domain/guide_injection_domaine.md](.harness/knowledge/domain/guide_injection_domaine.md).
