# Core C : Arithmétique de Pointeurs & Catalogue des Comportements Indéterminés (UB)

> Ce document liste les pièges fatals en langage C où le compilateur est autorisé à supprimer du code ou corrompre la mémoire.

---

## 1. La Règle d'Or de l'Arithmétique de Pointeurs

En C, un pointeur n'est pas un simple nombre entier : il possède une **provenance** rattachée à un objet alloué.
- Il est strictement interdit d'incrémenter un pointeur au-delà de `base + taille`.
- Seule l'adresse située immédiatement un cran après le dernier élément (`base + taille`) est tolérée pour les comparaisons de fin de boucle, mais elle ne doit **jamais** être déréférencée.

---

## 2. Le Catalogue Noir des Undefined Behaviors (UB) en C

| UB Fréquent | Ce que fait le compilateur / processeur | Règle de prévention |
|---|---|---|
| **Débordement d'entier signé** | Supprime des vérifications `if (x + 1 > x)` car le compilateur assume qu'un signé ne déborde jamais. | Utiliser des entiers non signés (`size_t`, `uint32_t`) ou vérifier avant d'ajouter. |
| **Violation de Strict Aliasing** | Accéder à la mémoire d'un `int` via un pointeur `float*`. LLVM réordonne les lectures/écritures et produit des données fausses. | Utiliser `memcpy` ou `union` dédiée pour le bit-casting. |
| **Déréférencement non aligné** | Crash processeur sur ARM / bus error, ou ralentissement sur x86. | Respecter l'alignement naturel du type (`alignof`). |
| **Lecture de variable non initialisée** | Valeur indéterminée, fuite d'informations mémoire de la pile. | Toujours initialiser à la déclaration (`int x = 0;`). |
| **Shift supérieur à la taille du type** | `x << 32` sur un entier 32-bit est un UB absolu. | Vérifier la borne du décalage (`shift < 32`). |

---

## 3. Checklist Actionnable pour l'Agent

- [ ] Tout pointeur utilisé provient-il d'un objet vivant avec une provenance valide ?
- [ ] Aucune conversion sauvage de type via cast direct de pointeur incompatible (respect du strict aliasing) ?
- [ ] Toutes les variables locales sont-elles explicitement initialisées dès leur déclaration ?
- [ ] Tout décalage de bits (*bit shift*) est-il borné par la taille du type en bits ?
