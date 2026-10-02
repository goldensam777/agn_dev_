# 02. Analyse Syntaxique & Grammaires Formelles

> Synthèse algorithmique issue du Dragon Book (Chapitre 4 : *Syntax Analysis*). De la théorie des grammaires non-contextuelles aux parsers modernes de Pratt et LR/LALR.

---

## 1. Grammaires Non-Contextuelles (CFG)

Une grammaire est un quadruplet $G = (V, \Sigma, R, S)$ où :
- $V$ : ensemble des non-terminaux (variables).
- $\Sigma$ : ensemble des terminaux (tokens).
- $R$ : ensemble de règles de production $A \to \alpha$ où $A \in V$ et $\alpha \in (V \cup \Sigma)^*$.
- $S \in V$ : axiome de départ.

---

## 2. Calcul des Ensembles $\text{FIRST}$ & $\text{FOLLOW}$

Ces deux fonctions sont indispensables à la construction des tables prédictives LL(1) et des automates LR.

### Définition & Calcul de $\text{FIRST}(\alpha)$ :
Pour une chaîne de symboles $\alpha$ :
1. Si $X$ est un terminal, $\text{FIRST}(X) = \{X\}$.
2. Si $X \to \epsilon$ est une production, alors $\epsilon \in \text{FIRST}(X)$.
3. Si $X$ est un non-terminal et $X \to Y_1 Y_2 \dots Y_k$ :
   - Ajouter $\text{FIRST}(Y_1) \setminus \{\epsilon\}$ à $\text{FIRST}(X)$.
   - Si $\epsilon \in \text{FIRST}(Y_1)$, ajouter $\text{FIRST}(Y_2) \setminus \{\epsilon\}$, et ainsi de suite.
   - Si $\epsilon \in \text{FIRST}(Y_i)$ pour tout $1 \le i \le k$, alors $\epsilon \in \text{FIRST}(X)$.

### Définition & Calcul de $\text{FOLLOW}(A)$ :
Pour un non-terminal $A$ :
1. Ajouter le symbole de fin d'entrée $\$$ à $\text{FOLLOW}(S)$.
2. Si une production $A \to \alpha B \beta$ existe, tout symbole de $\text{FIRST}(\beta) \setminus \{\epsilon\}$ est dans $\text{FOLLOW}(B)$.
3. Si une production $A \to \alpha B$ existe, ou $A \to \alpha B \beta$ où $\epsilon \in \text{FIRST}(\beta)$, alors tout symbole de $\text{FOLLOW}(A)$ est dans $\text{FOLLOW}(B)$.

---

## 3. Analyse Descendante Prédictive LL(1) & Pratt Parser

### Table d'Analyse LL(1) :
Pour chaque production $A \to \alpha$ :
- Pour chaque terminal $a \in \text{FIRST}(\alpha)$, ajouter $A \to \alpha$ dans $M[A, a]$.
- Si $\epsilon \in \text{FIRST}(\alpha)$, ajouter $A \to \alpha$ dans $M[A, b]$ pour tout $b \in \text{FOLLOW}(A)$.

> **Propriété LL(1) :** Une grammaire est LL(1) si et seulement si aucune case de la table $M[A, a]$ ne contient plus d'une production.

### L'Algorithme de Pratt (Top-Down Operator Precedence) :
Utilisé par Clang et rustc pour les expressions arithmétiques et binaires :
```python
def parse_expression(min_binding_power: int) -> ASTNode:
    token = advance()
    left_node = nud(token)  # Null Denotation (littéraux, unaires)

    while True:
        lookahead = peek()
        if lookahead == EOF or binding_power_left(lookahead) < min_binding_power:
            break
        token = advance()
        left_node = led(token, left_node)  # Left Denotation (opérateurs binaires, appels)

    return left_node
```

---

## 4. Analyse Ascendante LR (Bottom-Up)

Les parsers LR lisent l'entrée de gauche à droite et construisent une dérivation droite inversée (*handle pruning*).

| Type de Parser LR | Nombre d'États | Puissance | Lookahead |
|---|---|---|---|
| **LR(0)** | Minimal | Faible (aucune distinction de contexte) | Aucun |
| **SLR(1)** | Identique à LR(0) | Intermédiaire (utilise $\text{FOLLOW}$) | 1 symbole |
| **LALR(1)** | Identique à LR(0) (fusion des cœurs) | Standard industriel (Yacc, Bison) | 1 symbole |
| **LR(1) Canonique** | Très grand ($10\times$ à $100\times$ LR(0)) | Maximale pour grammaires non-ambiguës | 1 symbole précis par item |

### Fermeture d'Items LR(0) :
Si $[A \to \alpha \cdot B \beta] \in I$, alors pour chaque règle $B \to \gamma$, ajouter $[B \to \cdot \gamma]$ à $I$.

### Conflits LR :
- **Shift / Reduce :** L'analyseur hésite entre empiler le token courant ou réduire un ensemble de symboles déjà sur la pile.
- **Reduce / Reduce :** Deux règles de production différentes peuvent être réduites simultanément avec le même lookahead (souvent symptomatique d'une grammaire ambiguë ou mal factorisée).
