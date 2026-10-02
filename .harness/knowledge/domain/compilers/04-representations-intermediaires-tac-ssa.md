# 04. Représentations Intermédiaires : TAC & Forme SSA

> Synthèse algorithmique issue du Dragon Book (Chapitre 6 : *Intermediate-Code Generation*). Du Code à Trois Adresses (TAC) à la forme SSA (Static Single Assignment) utilisée par LLVM.

---

## 1. Pourquoi une Représentation Intermédiaire (IR) ?

L'IR découple le frontend du backend : pour $M$ langages sources et $N$ architectures cibles (x86_64, ARM64, RISC-V, WebAssembly), l'IR réduit la complexité du compilateur de $M \times N$ à $M + N$.

---

## 2. Code à Trois Adresses (Three-Address Code - TAC)

Chaque instruction contient au maximum un opérateur et trois opérandes :
$$x = y \text{ op } z \quad \text{ou} \quad x = \text{op } y \quad \text{ou} \quad \text{if } x \text{ relop } y \text{ goto } L$$

### Formats de Stockage Canoniques :
1. **Quadruplets :** `(op, arg1, arg2, result)`. L'emplacement explicite de `result` facilite grandement le réordonnancement d'instructions et les optimisations.
2. **Triplets :** `(op, arg1, arg2)`. Le résultat est implicitement la position de l'instruction dans le tableau d'IR (évite les noms de temporaires, mais rend les déplacements d'instructions complexes).

---

## 3. La Forme SSA (Static Single Assignment)

Dans la forme SSA (standard universel de LLVM IR, GCC GIMPLE et Go) :
1. **Chaque variable est assignée exactement une seule fois.**
2. **Chaque utilisation d'une variable est dominée par sa définition.**

### Nœuds $\phi$ (Phi Functions) :
Aux points de convergence du flot de contrôle (recombinaison de branches `if/else` ou en-têtes de boucles), une fonction $\phi$ sélectionne la valeur de la variable en fonction du bloc prédécesseur emprunté :
$$x_3 = \phi(x_1, x_2)$$

---

## 4. Algorithme de Cytron pour le Placement Optimal des Fonctions $\phi$

Pour placer les fonctions $\phi$ de façon minimale :

### 1. Arbre de Dominance :
Un nœud $d$ domine un nœud $n$ ($d \text{ dom } n$) si tout chemin depuis le point d'entrée du programme vers $n$ passe obligatoirement par $d$.
- $\text{idom}(n)$ est le dominateur immédiat strict le plus proche de $n$.

### 2. Frontière de Dominance ($\text{DF}$) :
La frontière de dominance d'un nœud $X$ est l'ensemble de tous les nœuds $Y$ tels que $X$ domine un prédécesseur de $Y$, mais $X$ ne domine pas strictement $Y$ :
$$\text{DF}(X) = \{ Y \mid \exists P \in \text{Pred}(Y) \text{ t.q. } X \text{ dom } P \text{ et } X \text{ ne domine pas strictement } Y \}$$

### 3. Règle de Placement :
Si une variable $v$ est assignée dans un bloc de base $B$, alors une fonction $\phi$ pour $v$ doit être insérée dans chaque bloc appartenant à la **Frontière de Dominance Itérée** $\text{IDF}(B)$.

---

## 5. Renommage des Variables SSA

Une fois les fonctions $\phi$ insérées :
- Un parcours en profondeur de l'arbre de dominance maintient une pile de compteurs de versions pour chaque variable d'origine.
- Chaque définition $x = \dots$ génère une nouvelle version $x_k$ (incrément du compteur, empilement).
- Chaque lecture de $x$ est remplacée par la version en sommet de pile.
- En sortant du sous-arbre de dominance, les versions locales sont dépilées pour restaurer la portée parente.
