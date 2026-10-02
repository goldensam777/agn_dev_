# Core TS : Typage Nominal par "Branded Types"

> Ce document détaille comment surpasser le typage structurel de TypeScript pour éliminer la Primitive Obsession avec un surcoût runtime strictement nul ($0$ overhead).

---

## 1. Le Problème du Typage Structurel en TypeScript

Par défaut, TypeScript compare les types par leur structure interne (*duck typing*) :
```typescript
type UserId = string;
type OrderId = string;

function cancelOrder(userId: UserId, orderId: OrderId) { ... }

const u: UserId = "usr_123";
const o: OrderId = "ord_456";

// ❌ BOGUE INVISIBLE : TypeScript accepte l'inversion car les deux sont de simples 'string' !
cancelOrder(o, u);
```

---

## 2. Le Mécanisme de Branding avec `unique symbol`

En attachant une propriété fantôme indexée par un symbole unique, le type devient nominal et empêche toute confusion :

```typescript
declare const BrandSymbol: unique symbol;

export type Branded<T, BrandName extends string> = T & {
  readonly [BrandSymbol]: BrandName;
};

// Définition des types nominaux
export type UserId = Branded<string, "UserId">;
export type OrderId = Branded<string, "OrderId">;
export type Meters = Branded<number, "Meters">;
export type Seconds = Branded<number, "Seconds">;

// Constructeur de validation (Smart Constructor)
export function parseUserId(raw: string): UserId {
  if (!raw.startsWith("usr_")) {
    throw new Error(`Format UserId invalide : ${raw}`);
  }
  return raw as UserId;
}

// Maintenant :
// cancelOrder(o, u); // ❌ ERREUR DE COMPILATION IMMÉDIATE !
```

---

## 3. Coût Runtime : Zéro

À la compilation, le `BrandSymbol` n'existe pas en JavaScript. Le code généré est une chaîne ou un nombre natif pur. Le branding offre une **sécurité absolue sans aucun coût de mémoire ni de CPU**.

---

## 4. Checklist Actionnable pour l'Agent

- [ ] Les identifiants système (ID utilisateur, ID de job, hashs) sont-ils typés avec des *Branded Types* ?
- [ ] Les grandeurs physiques ou unités incompatibles (degrés vs radians, secondes vs millisecondes) sont-elles distinctes via branding ?
- [ ] Les branded types sont-ils instanciés via des constructeurs validants (*smart constructors*) ?
