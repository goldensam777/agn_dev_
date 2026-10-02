# Domaine JS : Optimisation JIT (TurboFan) & Éléments de Tableaux V8

> Ce document détaille les règles d'agencement interne des tableaux pour éviter les déoptimisations du compilateur TurboFan.

---

## 1. Les Types d'Éléments dans V8 : PACKED vs HOLEY

V8 classe les tableaux selon deux axes fondamentaux :
- **PACKED (Dense) :** Tous les indices sont contigus et occupés.
- **HOLEY (À trous) :** Il existe des indices non définis entre le début et la fin.

### Le Piège du Tableau à Trous (Holey Array) :
```javascript
// ❌ CRIME JIT : Création d'un tableau à trous
const arr = [];
arr[100] = 42; // Crée 100 trous !
// V8 doit désormais vérifier la chaîne de prototypes à chaque accès arr[i].
// Ralentissement de 5x à 10x sur toutes les boucles !

// ✓ BON : Tableau dense sans trous
const dense = new Array(100);
dense.fill(0); // Rempli explicitement
```

---

## 2. SMI vs DOUBLE vs OBJECTS

- **PACKED_SMI_ELEMENTS (Le plus rapide) :** Petits entiers 31-bit. Stockés directement dans le pointeur sans allocation sur le tas (*Small Integer*).
- **PACKED_DOUBLE_ELEMENTS :** Nombres flottants.
- **PACKED_ELEMENTS :** Objets ou types mixtes.
- **Règle absolue :** Ne jamais mélanger des entiers, des flottants et des chaînes dans un même tableau critique.

---

## 3. Checklist Actionnable pour l'Agent

- [ ] Les tableaux sont-ils denses (*packed*) sans trous d'indices non initialisés ?
- [ ] Les collections critiques contiennent-elles un seul type homogène de données ?
- [ ] Les fonctions des boucles chaudes reçoivent-elles toujours des arguments de même type ?
