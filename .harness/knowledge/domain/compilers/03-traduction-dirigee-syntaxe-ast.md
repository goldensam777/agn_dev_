# 03. Traduction Dirigée par la Syntaxe & Construction d'AST

> Synthèse algorithmique issue du Dragon Book (Chapitre 5 : *Syntax-Directed Translation*). Formalisation des attributs sémantiques, graphes acycliques (DAG) et arbres de syntaxe abstraite.

---

## 1. Définitions Dirigées par la Syntaxe (SDD)

Une SDD associe à chaque symbole de grammaire un ensemble d'**attributs**, et à chaque règle de production un ensemble de **règles sémantiques**.

### Deux Classes d'Attributs Fondamentales :
1. **Attributs Synthétisés :** La valeur d'un nœud est calculée uniquement à partir des attributs de ses nœuds enfants (ou de sa propre valeur lexicale).
   - *Grammaire S-attribuée :* Ne contient **que** des attributs synthétisés.
   - *Propriété clé :* Évaluable de bas en haut (*bottom-up*) en une seule passe durant l'analyse LR sans construire d'arbre en mémoire.
2. **Attributs Hérités :** La valeur d'un nœud est calculée à partir des attributs de ses parents ou de ses frères aînés (à gauche).
   - *Grammaire L-attribuée :* Les dépendances d'héritage vont strictement de gauche à droite.
   - *Propriété clé :* Évaluable en un seul parcours profondeur d'abord (*depth-first pre-order*), parfaitement compatible avec les parsers descendants LL(1) et récursifs.

---

## 2. Arbre Concret (CST) vs Arbre Abstrait (AST)

- **Parse Tree (CST - Concrete Syntax Tree) :** Reflète chaque étape de la grammaire formelle, incluant les parenthèses, virgules, mots-clés de ponctuation et nœuds intermédiaires inutiles (ex: `Expr -> Term -> Factor -> Number`).
- **Abstract Syntax Tree (AST) :** Élimine le bruit syntaxique pour ne conserver que la structure sémantique et les opérandes :
  $$\text{CST}(a + b) \xrightarrow{\text{Abstraction}} \text{BinOpNode}(\text{Op}=\text{Add}, \text{Left}=\text{Ident}(a), \text{Right}=\text{Ident}(b))$$

---

## 3. Graphes Acycliques Dirigés (DAG) pour les Expressions

Le Dragon Book formalise la construction d'un DAG pour détecter immédiatement les sous-expressions communes au moment de la génération syntaxique.

### Algorithme de Numérotation de Valeur (*Value Numbering*) :
Chaque nœud est identifié par un hash unique $(\text{Op}, \text{LeftIndex}, \text{RightIndex})$ :
```python
class DAGBuilder:
    def __init__(self) -> None:
        self.node_table: dict[tuple[str, int, int], int] = {}
        self.nodes: list[tuple[str, int, int]] = []

    def get_or_create(self, op: str, left: int, right: int) -> int:
        key = (op, left, right)
        if key in self.node_table:
            return self.node_table[key]  # Réutilisation immédiate (CSE précoce)

        index = len(self.nodes)
        self.nodes.append(key)
        self.node_table[key] = index
        return index
```

---

## 4. Règle Industrielle de Gestion Mémoire des Nœuds AST

Dans la Forge :
- **Interdiction absolue :** Des `new` / `delete` individuels ou des `std::shared_ptr<ASTNode>` en C++, ou des `Rc<RefCell<ASTNode>>` en Rust.
- **Obligation :** Utiliser une **Arena de mémoire contiguë** (allocateur linéaire *Bump Allocator*).
  - Allocation en $O(1)$ par incrément d'un pointeur de tête.
  - Zéro fragmentation mémoire, saturation du cache L1/L2.
  - Libération globale en $O(1)$ à la fin de la compilation par simple reset du pointeur d'arène.
