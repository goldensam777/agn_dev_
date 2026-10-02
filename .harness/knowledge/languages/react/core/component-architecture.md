# Core React : Architecture de Composants & Typage des Props

> Ce document établit les règles d'architecture et de composition des composants React.

---

## 1. Séparation Stricte : Conteneur vs Présentation

1. **Composants de Présentation (UI Pures) :**
   - Ne connaissent ni le réseau, ni le serveur, ni les schémas Zod.
   - Reçoivent leurs données par `props` et émettent des événements via des callbacks typés (`onSelect`, `onChange`).
2. **Composants Conteneurs / Pages :**
   - Gèrent la récupération de données (`fetch`), l'état global et la validation.
   - Délèguent le rendu visuel aux composants de présentation.

---

## 2. Typage Strict des `Props`

Ne jamais utiliser `any` ou laisser les props implicites :
```tsx
import React, { ReactNode } from "react";

export interface ScientificCardProps {
  title: string;
  metricValue: number;
  unit: string;
  isPositive?: boolean;
  children?: ReactNode;
  onRefresh?: () => void;
}

export function ScientificCard({
  title,
  metricValue,
  unit,
  isPositive = true,
  children,
  onRefresh,
}: ScientificCardProps) {
  return (
    <article className="card">
      <h3>{title}</h3>
      <div className={isPositive ? "text-green" : "text-red"}>
        {metricValue.toFixed(2)} {unit}
      </div>
      {children}
      {onRefresh && <button onClick={onRefresh}>Actualiser</button>}
    </article>
  );
}
```

---

## 3. Checklist Actionnable pour l'Agent

- [ ] Les composants de présentation sont-ils complètement découplés des appels réseau ?
- [ ] L'ensemble des props est-il explicitement défini dans une `interface` dédiée ?
- [ ] Les enfants personnalisables sont-ils typés avec `React.ReactNode` ?
