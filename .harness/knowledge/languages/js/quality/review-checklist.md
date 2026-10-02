# Quality JS : Grille de Revue & Recette Outillage Multi-Tier

> Ce document fournit la grille d'évaluation et les commandes de profilage pour le code JavaScript et V8.

---

## 1. Grille d'Évaluation & Système de Scoring

### Niveau 1 : Défauts Bloquants (Score 0 — Rejet Immédiat)
- Blocage de la boucle d'événements (> 16 ms sans cession).
- Fuite de mémoire avérée par rétention de scope ou timer non annulé.
- Promesse non capturée (*unhandledRejection*).
- Usage de `delete` sur un objet dans une boucle critique.

### Niveau 2 : Défauts Majeurs (Pénalité -2 par occurrence)
- Création de tableaux à trous (*holey arrays*).
- Copie de buffer volumineux sans utiliser la liste de transfert de `postMessage`.
- Comparaison non stricte `==` au lieu de `===`.
- Usage de `slice()` au lieu de `subarray()` sur des buffers binaires.

### Niveau 3 : Défauts Mineurs (Pénalité -1 par occurrence)
- Utilisation de `_field` au lieu de `#field` pour l'encapsulation privée.
- Absence d'utilisation de `structuredClone` pour la copie profonde.
- Usage de `for...in` pour parcourir un tableau indicé.

---

## 2. Règle de la Review Citante

Chaque rejet de code doit citer formellement le document et paragraphe correspondant :
- Exemple : *"Rejet : Bloquant selon `.harness/knowledge/languages/js/core/event-loop-libuv.md §2` (la boucle de traitement par lots ne cède jamais la main avec setImmediate, provoquant une famine d'I/O)."*

---

## 3. Recette Outillage Multi-Tier & Profilage V8

```bash
# Tier 1 : Tests unitaires
npm test

# Tier 2 : Détection des déoptimisations TurboFan
node --trace-deopt --trace-ic mon_script.js

# Tier 3 : Traçage de la pression sur le ramasse-miettes
node --trace-gc --trace-gc-verbose mon_script.js
```
