# Domaine Fullstack : Performance & Plateforme de Production

> Base de connaissances transverse pour l'ingénierie, l'optimisation et l'exploitation des applications fullstack modernes (React, Node.js, TypeScript) en environnement de production haute disponibilité.

---

## 1. Organisation du Domaine

| Fichier | Sujet Traité |
|---|---|
| [`app-performance.md`](app-performance.md) | Boucle de mesure empirique, Core Web Vitals, latence p99, Event Loop delay, optimisation I/O et frontière Web↔Serveur. |
| [`platform-production.md`](platform-production.md) | Architecture sans état, cycle de vie (liveness/readiness, arrêt gracieux), résilience (breaker, bulkhead), observabilité (RED/USE, OpenTelemetry), conteneurs durcis. |

---

## 2. Règles de Chargement (Matrice de Routage)

| Phase / Tâche | Fichier à charger |
|---|---|
| Optimisation du temps de réponse API, requêtes SQL/NoSQL, réduction de bundle React | `fullstack/app-performance.md` |
| Conception de l'architecture serveur, déploiement conteneurisé, gestion des pannes, monitoring | `fullstack/platform-production.md` |
| Revue de code d'une nouvelle route ou fonctionnalité fullstack | `fullstack/app-performance.md` + `fullstack/platform-production.md` |

---

## 3. Règle Fondamentale de la Forge

> **Mesurer d'abord, optimiser ensuite :** Aucune modification de code visant la performance ne doit être acceptée sans métrique chiffrée avant/après (profilage CPU/mémoire, latence p95/p99 mesurée sur banc).
