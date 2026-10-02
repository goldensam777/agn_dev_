# Core TS : Validation sans Faille & Architecture de Contrats (Zod)

> Ce document établit le principe de Zero-Trust aux frontières logicielles du projet.

---

## 1. Le Principe Zero-Trust : `unknown` par Défaut

En TypeScript, les types n'existent plus à l'exécution. Faire ceci est une faute majeure :
```typescript
// ❌ CRIME MAJEUR EN TS : Faire confiance aveuglément au réseau
const data = (await res.json()) as UserProfile; // Bogue silencieux si l'API change !
```

Toute donnée entrant depuis :
- Une requête HTTP ou WebSocket
- Un fichier JSON lu sur le disque
- Un message IPC émis par le binaire natif C++

**DOIT être typée `unknown` jusqu'à validation par un schéma Zod dans `contracts/schemas.ts`.**

---

## 2. Inférence de Types depuis les Schémas

La source unique de vérité est le schéma de validation, le type TypeScript en découle automatiquement :
```typescript
import { z } from "zod";

export const JobConfigSchema = z.object({
  jobId: z.string().uuid(),
  algorithm: z.enum(["monte_carlo", "vector_dot", "fft"]),
  timeoutSeconds: z.number().int().positive().default(30),
});

// Le type TS est généré par inférence, garantissant une synchronisation à 100%
export type JobConfig = z.infer<typeof JobConfigSchema>;
```

---

## 3. `parse()` vs `safeParse()`

- Utiliser `safeParse()` pour les routes d'API afin de renvoyer un code HTTP 400 détaillé sans lever d'exception non gérée.
- Utiliser `parse()` uniquement lorsque l'échec de validation représente une corruption d'invariant interne devant stopper le traitement.

---

## 4. Checklist Actionnable pour l'Agent

- [ ] Tout appel réseau ou I/O externe est-il validé par un schéma Zod ?
- [ ] Aucun `as Type` n'est-il utilisé pour forcer le typage d'un payload JSON ?
- [ ] Les types partagés entre le serveur et le client dérivent-ils de `z.infer<...>` dans `contracts/` ?
