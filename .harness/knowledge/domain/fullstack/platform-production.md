# Plateformes Fullstack en Production

> À charger pour toute tâche « déployer, scaler, fiabiliser, monitorer ».
> Ce document répond à : *comment la plateforme survit-elle au lundi 9h, à la
> panne d'un nœud, et à la mauvaise release de vendredi soir ?*

---

## 1. Principes fondateurs

1. **Statelessness.** Les processus ne portent pas d'état en mémoire : tout état
   vit dans des stores externes (DB, cache, files). Un processus est jetable —
   on tue et on remplace, sans que personne ne le remarque.
2. **Configuration par environnement, jamais par branche.** 12-factor : config en
   variables d'env, secrets hors du code (gestionnaire de secrets), artefacts
   immuables (une image = un build = un digest).
3. **Prod-parity.** Le CI teste dans un environnement qui RESSEMBLE à la prod
   (mêmes versions, mêmes flags, mêmes limites). Règle déjà en place dans ce
   dépôt : `.github/workflows/ci.yml` exécute `scripts/verify.sh` — le verdict
   local et le verdict CI sont le MÊME verdict.
4. **Dégradation contrôlée.** Chaque dépendance externe a un comportement défini
   quand elle tombe (circuit breaker, valeurs de repli, files de reprise).

## 2. Cycle de vie d'un processus (souvent négligé, toujours facteur d'incident)

- **Démarrage :** rapide (< 30 s idéalement), healthcheck de *readiness* qui ne
  passe que quand le service est réellement prêt (connexions ouvertes, warm-up fait).
- **Arrêt gracieux (obligatoire) :** sur SIGTERM → arrêter d'accepter → *drain*
  des requêtes en cours (avec deadline) → fermer pools/fichiers → sortir.
  Un processus tué proprement ne coupe aucune requête en vol.
- **Healthchecks distincts :** *liveness* (« le process est-il vivant ? » → redémarrer)
  et *readiness* (« peut-il recevoir du trafic ? » → retirer du load balancer).
  Ne jamais mélanger les deux.

## 3. Fiabilité : la boîte à outils (dans l'ordre de simplicité)

| Outil | Règle | Erreur qu'il évite |
|---|---|---|
| **Timeout partout** | Tout appel externe (HTTP, DB, file, binaire natif) a un timeout explicite. Jamais l'infini par défaut. | Une dépendance lente qui fige tout l'arbre |
| **Retry borné** | Backoff exponentiel + jitter, compte maximal (typ. 3-5). Idempotent OU clé d'idempotence. | Double-effet de bord + retry storm |
| **Circuit breaker** | Après N échecs consécutifs, ouvrir le circuit et échouer vite pendant une fenêtre | Propagation d'avalanche |
| **Bulkhead** | Pools séparés par dépendance : la file DB saturée ne doit pas priver la file du cache | Épuisement partagé des ressources |
| **Load shedding** | Sous surcharge : rejeter (429) préférable à s'effondrer plus tard | OOM + latence explosive |
| **File d'attente** | Tout travail > ~100 ms ou asynchrone par nature passe par une file, pas par la requête HTTP | Une requête qui survit à son client |

**Cas de la forge :** le calcul natif (bridge C++) DOIT passer par une file de
jobs avec un worker pool — jamais exécuté dans le handler Express. Le handler
valide (Zod), pose le job, répond immédiatement (202 + jobId) ; le worker exécute
et publie le résultat. (Schémas déjà prêts dans `contracts/schemas.ts` : `jobId`,
`status`, `resultChecksum`.)

## 4. Observabilité : voir la plateforme de l'extérieur

1. **Logs structurés** (JSON), avec `requestId`/`jobId` propagé, et *redaction*
   systématique des secrets/données personnelles. Un log qu'on ne peut pas
   agréger ne sert à rien.
2. **Métriques RED par service :** Rate (req/s), Errors (ratio), Duration
   (p50/p99). Métriques USE par ressource : Utilization, Saturation, Errors.
3. **Traces distribuées** (OpenTelemetry) : une requête utilisateur = une trace
   traversant web → serveur → file → worker → binaire natif. Sans trace, le p99
   n'est pas un chiffre, c'est un mystère.
4. **SLO + error budget :** objectifs explicites (ex. p99 < 300 ms glissant 30 j,
   disponibilité 99,9 %) ; n'alerter que sur symptômes utilisateurs — pas sur
   chaque seuil CPU.
5. **Santé native instrumentée :** le bridge expose sa disponibilité
   (`bridge.isHealthy()` déjà en place) — le moteur C++ est une métrique de la
   plateforme, pas une boîte noire.

## 5. Déploiement

- **CI = portail.** Tests + `verify.sh` + build image + scan (dépendances,
  secrets) : rien ne passe en prod sans le pipeline. Déploiement manuel = dette
  d'incident.
- **Stratégies :** canary (1-5 % du trafic, métriques comparées, auto-rollback)
  ou blue/green. **Rollback < 5 min**, répété en exercice (game day).
- **Migrations DB expand/contract :** ajouter compatible → déployer le code →
  retirer l'ancien. Jamais de migration qui casse la version N-1 en cours de
  rollout.
- **Feature flags** pour tout ce qui est risqué : désactiver sans redeploy.

## 6. Conteneurs durcis (notre `.forge/Dockerfile` suit déjà l'esprit)

- Utilisateur non-root, filesystem read-only si possible, capabilities droppées.
- Limites CPU/RAM explicites (le `docker-compose.yml` les fixe déjà).
- Images épinglées par digest, jamais de tag `latest` en prod.
- Aucun secret dans l'image ou le dépôt.

## 7. Sécurité de production (le minimum non négociable)

- TLS partout, headers de sécurité (CSP, HSTS), CORS restrictif (origines nommées).
- Rate limiting par IP/clé d'API en amont (token bucket).
- Audit des dépendances dans le CI (`npm audit`, `cargo audit`, Dependabot/Renovate),
  lockfiles préservés, SBOM généré par build.

## 8. Capacité : scaler sur des signaux qui comptent

- **Load testing régulier** (k6) — mesurer la capacité avant qu'elle soit nécessaire.
- **Autoscaling sur signaux métier :** profondeur de file, p99, latence de queue —
  le CPU seul est un mauvais proxy (une boucle bloquée peut afficher 100 % CPU
  en traitant moins).
- **Le calcul natif scale par workers horizontaux** derrière la file de jobs
  (lien avec `scientific-in-production.md`).

## Checklist « mise en prod d'une feature »

- [ ] Pas d'état en mémoire du processus (statelessness vérifiée)
- [ ] Timeouts explicites sur tous les appels externes
- [ ] Retry borné (+ idempotence si effet de bord)
- [ ] Healthchecks liveness/readiness distincts
- [ ] Graceful shutdown implémenté
- [ ] Logs structurés avec jobId/requestId
- [ ] Métrique RED ajoutée au dashboard
- [ ] Chemin de rollback identifié et chronométré
- [ ] `bash scripts/verify.sh` : exit 0
