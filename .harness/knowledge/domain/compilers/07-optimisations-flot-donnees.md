# 07. Analyse de Flot de Données & Optimisations Globales

> Synthèse algorithmique issue du Dragon Book (Chapitre 9 : *Machine-Independent Optimizations*). Cadre algébrique de point fixe, analyses canoniques et optimisations de boucles.

---

## 1. Le Cadre Algébrique d'Analyse de Flot de Données

Une analyse de flot de données résout un système d'équations de transfert sur le CFG par calcul de **point fixe** (Algorithme de Kildall / *Worklist Algorithm*) :

$$f_B(x) = \text{gen}_B \cup (x \setminus \text{kill}_B)$$

| Propriété | Analyse Avant (*Forward*) | Analyse Arrière (*Backward*) |
|---|---|---|
| **Sens de Propagation** | Des entrées vers les sorties | Des sorties vers les entrées |
| **Équation d'Entrée** | $\text{IN}[B] = \bigwedge_{P \in \text{Pred}(B)} \text{OUT}[P]$ | $\text{OUT}[B] = \bigwedge_{S \in \text{Succ}(B)} \text{IN}[S]$ |
| **Équation de Sortie** | $\text{OUT}[B] = f_B(\text{IN}[B])$ | $\text{IN}[B] = f_B(\text{OUT}[B])$ |

---

## 2. Les Trois Analyses Canoniques du Dragon Book

### 1. Définitions Atteignantes (*Reaching Definitions*) :
- **Sens :** Avant (*Forward*).
- **Opérateur de Confluence ($\wedge$) :** Union ($\bigcup$, s'il existe *au moins un* chemin où la définition survit).
- **Équation :** $\text{OUT}[B] = \text{gen}_B \cup (\text{IN}[B] \setminus \text{kill}_B)$.
- **Usage :** Propagation de constantes, construction des chaînes Def-Use.

### 2. Expressions Disponibles (*Available Expressions*) :
- **Sens :** Avant (*Forward*).
- **Opérateur de Confluence ($\wedge$) :** Intersection ($\bigcap$, l'expression doit être calculée sur *tous* les chemins sans que ses opérandes soient modifiés).
- **Équation :** $\text{OUT}[B] = e\_\text{gen}_B \cup (\text{IN}[B] \setminus e\_\text{kill}_B)$.
- **Usage :** Élimination globale des sous-expressions communes (Global CSE).

### 3. Variables Vivantes (*Live Variables*) :
- **Sens :** Arrière (*Backward*).
- **Opérateur de Confluence ($\wedge$) :** Union ($\bigcup$, une variable est vivante si elle est lue sur *au moins un* chemin futur avant d'être redéfinie).
- **Équation :** $\text{IN}[B] = \text{use}_B \cup (\text{OUT}[B] \setminus \text{def}_B)$.
- **Usage :** Élimination du code mort et **fondation absolue de l'allocation de registres**.

---

## 3. Optimisations de Boucles Fondamentales

### A. Détection des Boucles Naturelles :
Une arête $n \to d$ dans le CFG est une **arête arrière (*back-edge*)** si et seulement si $d$ domine $n$ ($d \text{ dom } n$).
- $d$ est l'unique en-tête (*header*) de la boucle.
- La boucle naturelle associée est l'ensemble composé de $d$ et de tous les nœuds pouvant atteindre $n$ sans passer par $d$.

### B. Déplacement d'Invariants de Boucle (LICM - Loop-Invariant Code Motion) :
Une instruction $x = y + z$ dans une boucle est un invariant si $y$ et $z$ sont soit des constantes, soit des définitions extérieures à la boucle, soit des invariants déjà identifiés.
- **Conditions de déplacement vers le pré-en-tête (*pre-header*) :**
  1. Le bloc de l'instruction domine toutes les sorties de la boucle.
  2. Aucune autre instruction de la boucle n'assigne $x$.
  3. Toutes les utilisations de $x$ dans la boucle ne sont alimentées que par cette définition.

### C. Variables d'Induction & Réduction de Puissance :
Si $i$ est le compteur d'une boucle ($i = i + c$), toute variable $j = a \times i + b$ est une variable d'induction qui peut être transformée en une addition récurrente $j = j + (a \times c)$, évitant toute multiplication matricielle ou d'indexation à chaque itération.
