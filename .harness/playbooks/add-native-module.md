# Playbook : Ajouter un Module Natif (C++ / Rust)

Ce playbook décrit la procédure standard pour ajouter un algorithme ou module haute performance dans `native/`.

---

## Étapes Obligatoires

### 1. Définir l'interface publique
- En C++ : Créer un header dans `native/include/mon_module.hpp`.
- En Rust : Exposer des fonctions publiques documentées dans `native/src/lib.rs` ou un sous-module.
- **Règle :** L'interface doit être pure, sans dépendances vers le framework web ou serveur.

### 2. Implémenter l'algorithme
- Écrire le code source dans `native/src/mon_module.cpp` ou `native/src/`.
- Garantir :
  - La clarté de la propriété mémoire (RAII, aucun pointeur brut non géré).
  - L'optimisation des accès mémoire (accès contigus, données alignées).

### 3. Écrire les tests unitaires
- Ajouter les cas de tests dans `native/tests/test_mon_module.cpp`.
- Couvrir les cas limites (vecteurs vides, très grandes valeurs, allocations extrêmes).

### 4. Ajouter le micro-benchmark (Mercuria)
- Créer un banc d'essai dans `native/bench/bench_mon_module.cpp` mesurant le débit (opérations par seconde) et la mémoire allouée.

### 5. Vérifier la conformité
Exécuter la compilation avec ASan et UBSan :
```bash
bash scripts/verify.sh
```
Aucun warning ni aucune erreur de mémoire ne doit être signalée.
