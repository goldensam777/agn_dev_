# 01. Analyse Lexicale & Automates Finis

> Synthèse algorithmique issue du Dragon Book (Chapitre 3 : *Lexical Analysis*). Fondements mathématiques pour les lexers haute performance à zéro-allocation.

---

## 1. Rôle & Pipeline de l'Analyseur Lexical

L'analyseur lexical (Lexer) convertit un flux continu de caractères d'entrée en une séquence ordonnée de **Tokens** :
$$\text{Source Code (chars)} \xrightarrow{\text{Lexer}} \langle\text{Token\_Kind}, \text{Lexeme}, \text{SourceSpan}\rangle$$

### Principes Fondamentaux :
1. **Règle du Plus Long Match (*Maximal Munch*) :** Entre deux préfixes valides (ex: `>` et `>=`), l'analyseur choisit toujours le lexème le plus long possible.
2. **Priorité des Mots-Clés sur les Identifiants :** Une table de hachage statique ou un automate déterministe teste en premier lieu si un lexème correspond à un mot-clé réservé (`fn`, `let`, `if`), sinon il est étiqueté `IDENTIFIER`.

---

## 2. Construction de Thompson (Regex $\to$ NFA)

Transforme toute expression régulière en un Automate Fini Non Déterministe (NFA) avec transitions-$\epsilon$ en temps linéaire $O(|r|)$.

### Règles de Construction :
- **Base :** Pour le symbole $\epsilon$, créer un état initial $i$ et final $f$ avec une transition $\epsilon$. Pour un caractère $a$, une transition étiquetée $a$.
- **Concaténation ($r = s \cdot t$) :** Relier l'état final du NFA de $s$ à l'état initial du NFA de $t$ par une transition-$\epsilon$.
- **Alternance ($r = s \mid t$) :** Créer un nouvel état initial pointant par $\epsilon$ vers les états initiaux de $s$ et $t$, et relier leurs états finaux par $\epsilon$ vers un nouvel état d'acceptation commun.
- **Étoile de Kleene ($r = s^*$) :** Créer un nouvel état initial et un nouvel état final reliés par $\epsilon$ permettant soit d'ignorer $s$ (0 occurrence), soit de boucler de la fin vers le début de $s$.

---

## 3. Déterminisation par Sous-Ensembles (NFA $\to$ DFA)

Convertit le NFA en un Automate Fini Déterministe (DFA) où chaque état du DFA représente un ensemble d'états du NFA.

### Algorithme de Fermeture-$\epsilon$ :
```python
def epsilon_closure(states: set[State]) -> set[State]:
    stack = list(states)
    closure = set(states)
    while stack:
        s = stack.pop()
        for target in s.epsilon_transitions:
            if target not in closure:
                closure.add(target)
                stack.append(target)
    return closure
```

### Construction des États du DFA :
1. État initial $D_0 = \text{epsilon\_closure}(\{s_0\})$.
2. Pour chaque état non marqué $T \subseteq S_{\text{NFA}}$ et chaque caractère $a \in \Sigma$ :
   $$U = \text{epsilon\_closure}\left(\bigcup_{s \in T} \text{move}(s, a)\right)$$
   Si $U \neq \emptyset$ et $U \notin \text{Dstates}$, ajouter $U$ comme nouvel état non marqué.
   Définir la transition $\text{Dtran}[T, a] = U$.

---

## 4. Minimisation de DFA : Algorithme de Hopcroft

Réduit le nombre d'états du DFA à son minimum canonique en complexité optimale $O(|S| \cdot |\Sigma| \log |S|)$.

### Principe :
1. Partitionner initialement les états en deux groupes : les états d'acceptation $F$ et les états de non-acceptation $S \setminus F$.
2. Raffiner la partition : si pour un groupe $P$ et un symbole $a$, les transitions mènent à deux sous-groupes distincts, scinder $P$ en sous-groupes équivalents.
3. Répéter jusqu'à stabilisation de la partition.

---

## 5. Gestion Mémoire & Buffering Zéro-Copie

Dans un compilateur industriel (Clang, rustc) :
- Ne jamais allouer une nouvelle chaîne de caractères pour chaque token.
- **Double Buffer avec Sentinelle :** Charger le code source par blocs de 4 Ko avec un caractère sentinelle (`\0` ou `EOF`) en fin de buffer pour éliminer la vérification de borne à chaque caractère lu dans la boucle interne.
- **Tokens en Span :** Chaque token conserve uniquement un pointeur brut ou un offset de début/fin (`std::span<const char>` ou `&str`) pointant directement dans le buffer source mappé en mémoire (`mmap`).
