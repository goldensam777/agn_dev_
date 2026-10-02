# 06. Génération de Code, Blocs de Base & Sélection d'Instructions

> Synthèse algorithmique issue du Dragon Book (Chapitre 8 : *Code Generation*). Partitionnement en blocs de base, graphes de flot de contrôle (CFG) et sélection d'instructions optimales.

---

## 1. Définition & Partitionnement en Blocs de Base

Un **Bloc de Base** est une séquence contiguë d'instructions à trois adresses pour laquelle :
- Le flot de contrôle entre obligatoirement par la première instruction.
- Le flot de contrôle sort obligatoirement par la dernière instruction sans possibilité de saut intermédiaire.

### Algorithme de Détection des « Leaders » :
1. La première instruction du programme à trois adresses est un leader.
2. Toute instruction qui est la cible d'un saut conditionnel ou inconditionnel (`goto L`) est un leader.
3. Toute instruction qui suit immédiatement un saut conditionnel ou inconditionnel est un leader.

> **Règle de découpage :** Pour chaque leader, son bloc de base s'étend de ce leader jusqu'à l'instruction précédant le prochain leader (ou la fin du programme).

---

## 2. Graphe de Flot de Contrôle (CFG - Control Flow Graph)

Le CFG est un graphe orienté $G = (V, E)$ où :
- Chaque sommet $B \in V$ est un bloc de base.
- Une arête $(B_1, B_2) \in E$ existe si le contrôle peut passer de la fin de $B_1$ au début de $B_2$ (par saut conditionnel, saut direct ou chute séquentielle / *fall-through*).

---

## 3. Optimisations Locales par DAG de Bloc de Base

À l'intérieur d'un bloc de base, un DAG permet d'appliquer 4 optimisations simultanées :
1. **Élimination des Sous-Expressions Communes Locales :** Si deux nœuds effectuent la même opération sur les mêmes entrées, le second est fusionné dans le premier.
2. **Élimination du Code Mort :** Tout nœud du DAG qui ne modifie aucune variable vivante en sortie de bloc est élagué.
3. **Identités Algébriques :** Simplification immédiate ($x + 0 \to x$, $x \times 1 \to x$, $x \times 0 \to 0$).
4. **Réduction de Puissance (*Strength Reduction*) :** Remplacer une multiplication coûteuse par un décalage de bits ($x \times 8 \to x \ll 3$).

---

## 4. Sélection d'Instructions par Programmation Dynamique

Le problème de la sélection d'instructions est modélisé par le recouvrement d'un arbre d'expressions par un ensemble de tuiles machines (*tree tiling*) :

### Algorithme d'Aho-Johnson :
1. **Passe ascendante (Bottom-Up) :** Calculer le coût minimal pour chaque nœud de l'arbre d'AST pour chaque classe d'instruction machine possible en mémorisant la règle de tuile optimale.
2. **Passe descendante (Top-Down) :** Émettre les instructions machines sélectionnées associées aux tuiles de coût minimal.

> **Maximal Munch (Glouton) :** Variante rapide consistant à choisir à chaque étape la plus grande tuile machine valide recouvrant le sommet de l'arbre.

---

## 5. Optimisations par Viseur (*Peephole Optimization*)

Passe finale opérant sur une fenêtre glissante de 2 à 4 instructions machines cibles :
- **Élimination des Redondances Load/Store :**
  ```asm
  MOV [rbp - 8], rax
  MOV rax, [rbp - 8]   ; <- Supprimé (rax contient déjà la valeur)
  ```
- **Élimination des Sauts vers Sauts :**
  ```asm
  JMP L1
  ...
  L1: JMP L2          ; <- Remplacer directement par JMP L2
  ```
