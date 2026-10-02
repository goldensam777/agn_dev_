# Domaine TS : Communication Fullstack, RPC & Ponts IPC

> Ce document établit les règles d'échange de données typées de bout en bout entre le client, le serveur et les moteurs natifs.

---

## 1. Typage Bout-en-Bout sans Codegen Lourd

Dans notre architecture monorepo, le client web et le serveur partagent directement `@forge/contracts` :
- Toute route serveur est indexée par une signature :
  ```typescript
  export interface ApiEndpoints {
    "POST /api/compute": {
      request: ComputeJobRequest;
      response: ComputeJobResponse;
    };
    "GET /api/health": {
      request: void;
      response: HealthCheckResponse;
    };
  }
  ```
- Le client d'API dans `web/src/api/client.ts` infère automatiquement les types des paramètres et du retour à partir de l'URL appelée.

---

## 2. Le Pont Natif IPC (Node.js $\leftrightarrow$ C++)

Dans `server/src/bridge/native_bridge.ts` :
1. **Entrée :** Les paramètres sont validés par Zod avant d'être passés au sous-processus.
2. **Sortie :** Le stdout JSON émis par le C++ est re-validé par Zod avant d'être retourné sous forme d'objet JavaScript typé.
3. **Isolation de Crash :** Si le binaire natif subit un *segfault*, le bridge capture le code de sortie non nul et le transforme en une erreur JavaScript exploitable sans faire tomber le serveur Node.js.

---

## 3. Checklist Actionnable pour l'Agent

- [ ] Tout appel API frontend utilise-t-il les types inférés depuis `contracts/` ?
- [ ] Le bridge natif valide-t-il la sortie du binaire avec le schéma Zod de réponse ?
- [ ] Les erreurs d'exécution du moteur natif sont-elles encapsulées dans des formats d'erreur normalisés ?
