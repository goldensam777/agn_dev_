# Domaine TS : Modélisation d'État & Immutabilité Structurelle

> Ce document fixe les règles de conception des états de données complexes en TypeScript.

---

## 1. Normalisation de l'État : Bannir les Listes Imbriquées

Pour gérer des collections d'entités (jobs, métriques, fichiers) :
- Ne jamais stocker des tableaux d'objets profondément imbriqués (`Job[]` contenant `Task[]` contenant `Log[]`).
- **Normaliser par identifiants :**
  ```typescript
  export interface NormalizedState {
    jobsById: Record<JobId, Job>;
    jobIds: JobId[];
    activeJobId: JobId | null;
  }
  ```
*Avantage : mise à jour d'un job en $O(1)$, sans recalculer ni recloner toute la hiérarchie.*

---

## 2. États Dérivés : Ne Jamais Dupliquer

- Ne jamais stocker dans l'état une valeur qui peut être calculée à partir d'autres propriétés (ex: stocker `list` et `listCount` séparément).
- Calculer les valeurs dérivées à la volée (via des sélecteurs purs ou `useMemo`).

---

## 3. Checklist Actionnable pour l'Agent

- [ ] L'état des collections est-il normalisé par clé d'identifiant (`byId`) ?
- [ ] Aucune donnée redondante ou calculable n'est-elle dupliquée dans l'état ?
- [ ] Les mises à jour d'état préservent-elles le partage structurel (*structural sharing*) ?
