# Core TS : Performance du Compilateur & Évitement de l'Explosion de Types

> Ce document traite de l'optimisation des temps de compilation et de la prévention des ralentissements de l'IDE.

---

## 1. Prévention de l'Explosion Combinatoire d'Unions

Les Template Literal Types ou les unions croisées peuvent générer des millions de types internes et paralyser le compilateur `tsc` :
```typescript
// ❌ DANGEREUX : 100 x 100 x 100 = 1 000 000 de types internes instanciés par le compilateur
type Method = ...; // 100 variantes
type Path = ...;   // 100 variantes
type Status = ...; // 100 variantes
type Route = `${Method}_${Path}_${Status}`;
```
*Si une union dépasse quelques centaines de variantes, décomposer en structures imbriquées.*

---

## 2. `interface` vs `type` pour la Performance du Compilateur

- **Pour les objets simples et les contrats :** Préférer `interface` à `type`.
- Le compilateur TypeScript met en cache les déclarations `interface` par leur nom (*canonical name cache*), ce qui accélère la vérification des types de 15 à 30% sur les gros dépôts.
- Réserver `type` aux unions, tuples, types primitifs et types conditionnels.

---

## 3. Configuration Optimale de `tsconfig.json`

- `"skipLibCheck": true` : Évite de revérifier les types de `node_modules` à chaque compilation.
- `"strict": true` : Active toutes les vérifications sans surcoût.
- `"exactOptionalPropertyTypes": true` : Empêche l'attribution accidentelle de `undefined` aux propriétés optionnelles.

---

## 4. Checklist Actionnable pour l'Agent

- [ ] Aucune récursion de type conditionnel ne risque-t-elle de dépasser la limite de profondeur du compilateur ?
- [ ] Les contrats d'objets standards utilisent-ils `interface` pour bénéficier du cache de `tsc` ?
- [ ] Le fichier `tsconfig.json` active-t-il `skipLibCheck: true` et l'ensemble des règles strictes ?
