# Domaine Fullstack : Résilience & Plateforme de Production

> Architecture sans état (Statelessness), gestion fine du cycle de vie des processus, boîte à outils de résilience, observabilité standardisée et durcissement des conteneurs.

---

## 1. Architecture Sans État (*Statelessness*)

Les serveurs API et orchestrateurs de la Forge doivent être strictement sans état :
- **Zéro Session en Mémoire Processus :** Aucun état d'authentification ou session utilisateur ne doit être stocké dans la mémoire locale d'une instance Node.js.
- **Stockage Centralisé :** Les états partagés, verrous distribués et jobs en file d'attente doivent résider dans une base dédiée (Redis / PostgreSQL / SQLite monté selon le cas).
- **Scalabilité Horizontale :** N'importe quelle instance peut être arrêtée ou redémarrée à tout instant sans provoquer d'interruption ni de perte d'état.

---

## 2. Gestion du Cycle de Vie des Processus

### A. Sondes d'Orchestration (Kubernetes / Docker) :
Ne jamais fusionner liveness et readiness :
- **Sonde de Liveness (`/health/liveness`) :** Vérifie que le processus Node.js n'est pas dans un état de deadlock. Si échec : redémarrage immédiat du conteneur.
- **Sonde de Readiness (`/health/readiness`) :** Vérifie que le serveur est prêt à recevoir du trafic (connexion DB établie, pont natif C++ opérationnel via `bridge.isHealthy()`). Si échec : retrait temporaire de la cible du load balancer sans redémarrage.

### B. Arrêt Gracieux (*Graceful Shutdown*) :
Lors de la réception du signal `SIGTERM` :
1. Marquer la sonde de *Readiness* à `false` pour couper le nouveau trafic entrant.
2. Cesser d'accepter de nouvelles requêtes HTTP.
3. Attendre la fin des requêtes en vol (*in-flight requests*) avec un timeout maximal (ex: 15 secondes).
4. Fermer proprement les connexions aux bases de données et terminer les processus natifs enfants C++/Rust.
5. Quitter avec le code de sortie `0`.

---

## 3. Boîte à Outils de Résilience

| Patron | Rôle | Implémentation / Règle |
|---|---|---|
| **Timeout Déterministe** | Empêcher une dépendance lente de bloquer un worker indéfiniment. | Tout appel sortant (HTTP, IPC natif, base de données) doit obligatoirement avoir un timeout explicite (ex: 5 000 ms). |
| **Retry avec Backoff Exponentiel & Jitter** | Éviter d'achever un service convalescent (*Thundering Herd*). | $T_{\text{wait}} = 2^{\text{attempt}} \times 100\text{ ms} + \text{random}(0, 50\text{ ms})$. Maximum 3 tentatives sur erreurs 5xx idempotentes. |
| **Circuit Breaker** | Couper immédiatement le trafic vers une ressource défaillante pour la laisser récupérer. | Après $N$ échecs consécutifs, basculer en état `OPEN` (échec immédiat sans appel pendant un temps de refroidissement). |
| **Bulkhead (Cloisonnement)** | Isoler les quotas de ressources entre fonctionnalités critiques et secondaires. | Les calculs intensifs sont limités à un pool fixe de workers pour ne jamais saturer l'API de commande. |
| **Load Shedding** | Protéger le serveur contre l'effondrement sous surcharge extrême. | Rejeter immédiatement avec `HTTP 503 Service Unavailable` dès que la file d'attente dépasse le seuil critique. |

---

## 4. Observabilité Standardisée

### A. Méthode RED pour les Services Web :
- **Rate :** Nombre de requêtes par seconde.
- **Errors :** Nombre de requêtes en échec (codes HTTP 5xx).
- **Duration :** Temps de réponse (distribution p50, p95, p99).

### B. Méthode USE pour l'Infrastructure :
- **Utilization :** Pourcentage de temps d'utilisation de la ressource (CPU, bande passante).
- **Saturation :** Longueur de la file d'attente de tâches en attente de la ressource.
- **Errors :** Nombre d'événements d'erreur de la ressource (défauts mémoire, paquets rejetés).

### C. Traces & Journaux Structurés :
- Logs émis au format **JSON structuré** sur `stdout` (jamais de fichiers locaux sans rotation).
- Propagation systématique d'un identifiant de corrélation (`X-Request-Id` / `traceparent` OpenTelemetry) entre le client Web, le serveur API et le sous-processus natif.

---

## 5. Durcissement des Conteneurs de Production

- **Utilisateur Non-Root :** L'application s'exécute sous un utilisateur dédié non privilégié (`USER node` ou `USER 1000`).
- **Système de Fichiers en Lecture Seule :** `readOnlyRootFilesystem: true` avec montage temporaire `tmpfs` uniquement sur `/tmp` si nécessaire.
- **Limites Strictes de Ressources :** Quotas CPU et mémoire explicites (`resources.limits.memory`, `resources.limits.cpu`).
