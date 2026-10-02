# Interfaces, Const-Correctness & Conception des Fonctions

> **Idée directrice** : Une interface est un **contrat**, pas une implémentation. La qualité
> d'une API se juge à ce qu'elle rend *impossible* plus qu'à ce qu'elle permet. En C++,
> `const`, les types et les signatures sont les briques du contrat — les commentaires ne sont
> que leur traduction pour humains.

---

## 1. Const-correctness : un contrat, pas une optimisation

`const` est le système de permissions du langage. Un programme de qualité l'utilise à
**tous les étages** :

```cpp
class Matrix {
public:
    double  operator()(int i, int j) const;   // lire : promesse "je ne modifie rien"
    double& operator()(int i, int j);         // écrire : mutation explicite
    // ...
};
```

- **La version `const` est la question que tout code se pose** : "puis-je l'appeler sur
  un objet que je ne possède qu'en lecture ?" Si une méthode ne mute pas l'état observable,
  elle doit être `const`. Une méthode non-const qui ne modifie rien est un mensonge qui
  propage des casts-away-const en aval.
- **Logical constness** : un membre `mutable` est légitime pour le cache interne
  (lazy evaluation, mémoïsation) — l'état *observable* ne change pas. Tout autre usage de
  `mutable` est suspect.
- **Const réflexe** : variables locales jamais réassignées → `const` (le compilateur
  attrape les écritures accidentelles, le lecteur voit l'intention).
- **Pointeurs** : maîtriser la grille `T*`, `const T*`, `T* const`, `const T* const`.
  Un paramètre `char* const` vs `const char*` ne veut pas dire la même chose ; confondre
  les deux est un bug hebdomadaire en revue.
- **`const_cast` pour écrire dans un objet const = UB** (sauf objet réellement non-const
  sous-jacent). Le besoin d'un `const_cast` est presque toujours un défaut de design
  remontant plus haut.

**Pour l'agent** : ne pas écrire un `const` parce que "ça compile" — chaque `const` est une
promesse vérifiée par le compilateur. Omettre un `const` légitime est une omission de qualité.

---

## 2. Conception des fonctions (Core Guidelines F.*)

### 2.1 Une fonction = une opération logique

- F.1, F.2, F.3 : nommage qui dit *quoi*, pas *comment* ; une seule opération logique ;
  court et simple. Si tu ne peux pas nommer la fonction sans "and", c'est deux fonctions.
- F.4 : si une fonction *peut* être évaluée à la compilation → `constexpr` (gratuit,
  élargit l'usage).
- F.6 : si une fonction ne doit pas thrower → `noexcept` (documente + optimise).
- F.9 : paramètre inutilisé → le laisser sans nom (documente l'intention).
- F.20/F.21 : **les sorties se retournent, elles ne se passent pas en paramètre sortie** :
  ```cpp
  // ❌ sortie déguisée en paramètre — le caller ne sait pas que result est écrit
  void stats(const Data& d, double& mean, double& var);
  // ✅ le type du retour raconte la forme du résultat
  struct Stats { double mean, variance; };
  Stats stats(const Data& d);
  ```

### 2.2 La table de passage des paramètres

(Détaillée dans ownership-raii.md §4 — la recopier mentalement avant d'écrire toute signature.)

Le point subtil : **passer un smart pointer en paramètre n'est jamais anodin**. Chaque type
de paramètre *est* une déclaration de sémantique de lifetime. `void f(Widget&)` accepte un
widget stack, heap, unique_ptr, shared_ptr — c'est la forme la plus générale et elle doit
être le défaut pour "j'utilise sans posséder".

### 2.3 Préconditions et postconditions

- I.5/I.7 : les fonctions non totales doivent documenter leurs préconditions ("i < size",
  "le vecteur n'est pas vide", "s non vide") — et la qualité monte d'un cran quand la
  précondition devient un type (cf. `not_null`, enum, `NonEmpty<Span>`).
- Les postconditions critiques (mutex relâché, invariant restauré) doivent être **tenues
  par des types** (lock_guard), pas par des commentaires (I.7).

---

## 3. Conception des interfaces de classes

### 3.1 Règles structurelles

- **I.1 Interfaces explicites** : éviter les conversions implicites trompeuses
  (`explicit` sur les ctors mono-paramètre), les paramètres adjacents interchangeables
  (I.24 : `(int rows, int cols)` appelé `(3, 3)` compile toujours — types forts ou
  tag dispatch si le risque est réel).
- **I.2 Pas de variables globales mutables** : cache caché, dépendances cachées, data
  races. Une constante globale est OK ; un état global est un défaut de design. (I.3 : les
  singletons sont des globals déguisés — préférer passer les dépendances en paramètre,
  le DI le plus simple qui soit.)
- **I.25** : pour une interface, préférer une classe abstraite avec des fonctions virtuelles
  pures ; réserver l'héritage de données aux cas réels de factorisation d'état.
- **I.27 PImpl** pour la stabilité d'ABI des bibliothèques : la classe publique ne contient
  qu'un `std::unique_ptr<Impl> pImpl_` ; l'implémentation bouge librement sans casser les
  binaires. Coût : une indirection — à réserver aux vraies frontières de bibliothèque.

### 3.2 La question héritage vs composition

Défaut = composition ("has-a"). Héritage public uniquement pour le sous-typage réel
(Liskov : tout usage de la base doit rester correct pour le dérivé). Un héritage pour
"réutiliser du code" est un anti-pattern classique ; préférer déléguer.

### 3.3 Encapsulation réelle

Private = "personne ne dépend de ça, je peux tout changer". Tout ce qui peut être privé
doit l'être ; les getters qui exposent des membres modifiables par référence non-const
cassent l'encapsulation sans rien gagner. Exposer un *comportement* (`area()`, `isEmpty()`),
pas une *structure*.

---

## 4. Types : la première ligne de défense

La plus grande qualité d'un programme est qu'il rende les états invalides **inexpressibles** :

| Au lieu de | Un type qui dit le vrai |
|---|---|
| `int angleDeg` | `class Radians { double v; explicit… }` |
| `std::string status` valant "ok"/"err" | `enum class Status { Ok, Err };` |
| `double price` pouvant être NaN | `class Price { double v; /* invariant v ≥ 0 */ };` |
| `int index` ou `-1` si absent | `std::optional<size_t>` |
| `bool, std::string` (paramètres liés) | struct de paramètres nommés |

- **`enum class` partout** : scope + pas de conversion implicite int. Un `enum` nu hérité
  du C est un trou dans le type system.
- **`std::optional`** pour la nullabilité explicite ; **`std::variant`** pour les sommes
  disciplinées ; **`std::strong_ordering`** pour les comparaisons totales (C++20).

---

## 5. Découpage et dépendances

- Un fichier = une responsabilité cohérente. Un header qui mélange définition et logique
  d'I/O, calcul et persistance est à scinder.
- **Dépendances minimales dans les headers** : forward declarations, `unique_ptr<Impl>`,
  pImpl — pour les temps de compilation et pour la lisibilité du graphe de dépendances.
- Inclure ce qu'on utilise (IWYU). Les `using namespace` dans les headers sont interdits.
- Fonctions pures privilégiées (F.8) : sans état caché → testables, parallélisables,
  raisonnables. L'état est précieux et coûteux ; il mérite une justification.

---

## 6. Checklist interfaces

- [ ] `const` partout où l'état observable n'est pas modifié ; aucun `const_cast` d'écriture
- [ ] Une fonction = une opération ; sorties retournées (struct si pluralité)
- [ ] Paramètres : sémantique de lifetime visible (`T&` défaut ; smart ptr = déclaration)
- [ ] Préconditions documentées ; les critiques sont des types (`span`, `not_null`, optionals)
- [ ] Pas de global mutable, pas de singleton — dépendances injectées
- [ ] Héritage = sous-typage réel ; composition par défaut ; PImpl aux frontières d'ABI
- [ ] États invalides inexpressibles (types forts, enum class, variant)
- [ ] Encapsulation : comportements exposés, structure cachée
