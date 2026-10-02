# Core TS : Système de Types Avancé & Type-Level Programming

> Ce document détaille l'usage de TypeScript pour prouver les invariants de conception à la compilation.

---

## 1. Types Conditionnels & Mot-Clé `infer`

Le mot-clé `infer` permet d'extraire des types imbriqués (comme le type de retour d'une promesse ou l'élément d'un tableau) sans duplication :
```typescript
// Extraction générique du type résolu d'une promesse
export type AwaitedResult<T> = T extends Promise<infer U> ? U : T;

// Extraction du payload d'une fonction de rappel
export type CallbackPayload<T> = T extends (payload: infer P) => void ? P : never;
```

---

## 2. Template Literal Types pour les Événements et Protocoles

Éliminer les chaînes magiques non typées en composant des types de littéraux textuels :
```typescript
type Entity = "job" | "worker" | "dataset";
type Action = "created" | "updated" | "failed";

// Génère automatiquement : "job:created" | "job:updated" | ... (9 combinaisons strictes)
export type SystemEvent = `${Entity}:${Action}`;
```

---

## 3. Types Mappés & Modificateurs (`readonly`, `-?`)

Pour construire des variantes strictes et immuables de modèles de données :
```typescript
// Rend tous les champs obligatoires et non mutables
export type DeepImmutable<T> = {
  readonly [K in keyof T]-?: T[K] extends object ? DeepImmutable<T[K]> : T[K];
};
```

---

## 4. Checklist Actionnable pour l'Agent

- [ ] Les types génériques sont-ils contraints avec `extends` plutôt que libres ?
- [ ] Les types dérivés exploitent-ils les types mappés plutôt qu'une redéclaration manuelle risquant la dérive ?
- [ ] Les chaînes de protocoles et événements exploitent-elles les *Template Literal Types* ?
