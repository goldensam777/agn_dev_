# Construction de Compilateurs & d'Interpréteurs en C++

> **Idée directrice** : Un compilateur a une exigence de qualité supérieure aux autres
> logiciels : un compilateur buggé ne plante pas — il produit du code **silencieusement
> faux**. La qualité = sémantique préservée (prouvée par tests sur lesquelles le langage
entier est projeté), diagnostics précis (c'est l'interface utilisateur du compilateur),
et itération rapide (fuzzing + golden tests à chaque changement).

---

## 1. Le pipeline : garder les phases séparées

```
source ──► lexer ──► parser ──► AST ──► [sémantique] ──► IR ──► [opt] ──► codegen ──► output
           tokens     structure   arbre    types/scope    interm.           cible
```

**Règle d'architecture** : chaque phase consomme le produit de la précédente et ne connaît
que lui. Le parser ne connaît pas les caractères (que les tokens) ; le codegen ne connaît
pas la syntaxe (que l'IR). Cette séparation est ce qui rend le compilateur testable phase
par phase — et c'est le premier critère de qualité.

## 2. Le lexer (tokenizer)

- **Un seul passage**, sur `std::string_view` de la source : zéro copie de chaînes.
- Token = `kind` (enum class) + `text` (string_view) + **position source** (offset, ligne,
  colonne) + valeur littérale si déjà parsée. La position source voyage dans *tous* les
  diagnostics jusqu'à l'utilisateur.
- Keywords vs identifiants : table de hachage, un seul lookup.
- Nombre maximal d'erreurs par fichier (p. ex. 10) pour ne pas noyer l'utilisateur.
- Le lexer est le hot path du frontend : simple, branch-minimal, pas d'allocation par token.

```cpp
enum class TokenKind { Identifier, Integer, Float, String, Plus, Minus, Star, If, Else, Eof /*…*/ };

struct Token {
    TokenKind kind;
    std::string_view text;
    std::uint32_t offset, line, column;
    // valeur littérale éventuelle dans un std::variant
};
```

## 3. Le parser : recursive descent + Pratt

**Choix recommandé pour un langage neuf** : descente récursive manuscrite, avec la
**precedence climbing / Pratt parsing** pour les expressions. C'est la méthode de Crafting
Interpreters (Nystrom), maintenable à vie, de meilleurs diagnostics que les parseurs
générés LALR, et suffisamment rapide.

```cpp
class Parser {
    Token peek() const; Token advance(); bool match(TokenKind k); Token expect(TokenKind k);
    // expressions : binding powers
    Expr* expression(int min_bp = 0);
    Expr* prefix();                       // littéraux, parenthèses, préfixes
    Expr* infix(Expr* lhs, TokenKind op); // binaires
};
// table : static constexpr int bp(TokenKind);  // +,- : 10 | *,/ : 20 | ^ : 30 (droit)
```

Règles de qualité :
- **Les opérateurs déclarent leur associativité dans la table**, pas dans du code spécial
  par opérateur. Ajouter un opérateur = une ligne.
- Le parser construit l'AST via **arena** (cf. memory.md) ou `unique_ptr` ; jamais de
  `new` nu dont personne ne connaît le propriétaire.
- **Récursion** : la descente récursive sature la pile sur les entrées adversariales
  (`((((((…))))))`). Prévoir une limite de profondeur explicite → erreur propre
  "expression trop profonde", pas un crash.

### Généreurs (quand les choisir)
- **re2c** : lexer générateur très rapide (C) — si le lexer devient le bottleneck mesuré.
- **flex/bison, ANTLR4 (cible C++)** : si l'équipe connaît, grammaire énorme, ou besoin
  de l'outil. Coût : toolchain supplémentaire, diagnostics plus durs à personnaliser.
- **Boost.Spirit X3, PEGTL** : parsers combinatoires/PEG header-only — bons pour DSL
  petits-moyens.

## 4. L'AST et la sémantique

### 4.1 Structure de l'AST
- Nœuds par `std::variant` (recommandé, visitable) ou hiérarchie avec `unique_ptr`.
- Chaque nœud garde sa **SourceLocation** (le diagnostic précis tient en une ligne).
- **Séparer AST "concret" (syntaxe) et IR (sémantique)** : l'AST est un reflet fidèle du
  texte ; l'IR est une forme normalisée (SSA) où les optimisations vivent. Ne pas faire
  d'optimisations sur l'AST.

### 4.2 Analyse sémantique (le vrai juge interne)
- Scope resolution, type checking, résolution de surcharge — avec **un mécanisme de
  diagnostics central** : liste de `(severity, message, location, notes)`, jamais de
  `cerr` dispersé dans les passes.
- Erreur sémantique ≠ crash : l'analyse continue après une erreur (autant que possible)
  pour rapporter *toutes* les erreurs d'un coup. La qualité d'un compilateur se mesure
  aussi aux erreurs qu'il n'a pas rapportées.

```cpp
struct Diagnostic {
    enum class Severity { Error, Warning, Note };
    Severity sev;
    SourceLocation loc;
    std::string message;
    std::string source_excerpt;   // la ligne avec un ^ sous l'endroit
};
```

### 4.3 Récupération d'erreurs du parser (panic mode)
Sur erreur de syntaxe : synchroniser sur un token sûr (`;`, `}`, fin de ligne selon le
langage), rapporter, continuer. Jamais de crash sur entrée invalide — l'entrée invalide
est le cas **normal** du compilateur.

## 5. IR, optimisation, codegen

- **SSA (Static Single Assignment)** comme forme IR par défaut : chaque valeur assignée
  une fois → simplifie analyses et optimisations. Les concepts à connaître : blocs de base,
  CFG, dominance, phi nodes.
- Ordre canonique des passes d'optimisation : d'abord les analyses (alias, const propagation),
  puis transformations simples (DCE, constant folding, inlining), puis le spécifique au
  langage. **Une passe = une responsabilité**, chacune testée isolément sur IR d'entrée/sortie.
- Codegen : soit un backend existant (**LLVM** : production-grade, mais poids et complexité ;
  **Cranelift** : JIT, compilation rapide ; **QBE, libfirm** : légers) soit maison (apprentissage,
  langages très spécifiques). **Ne pas écrire un backend x86 complet en production** — c'est
  des années de travail pour couvrir ce que LLVM fait déjà.
- Tests d'optimisation : chaque passe a des cas "input IR → output IR attendu" (golden
  tests), plus des tests end-to-end "programme → résultat".

## 6. Tests : la batterie qui rend le projet possible

| Test | Contenu |
|---|---|
| Lexer unit | chaque type de token, edge cases (escapes, unicode, nombres mal formés) |
| Parser unit | forme de l'AST attendue ; récupération d'erreur |
| Round-trip | pretty-print(AST) → reparse → même AST (attrape les pertes sémantiques) |
| **Golden / lit tests** | programmes + sortie attendue (exécutable ou IR) : `// RUN: %compiler %s | %FileCheck` |
| Diagnostics | chaque message d'erreur a un test qui vérifie message ET position |
| **Fuzzing** | **libFuzzer** sur lexer+parser (entrées aléatoires → pas de crash, pas d'UB sous ASan) ; corpus minimisé commité |
| Differential | comparer l'interpréteur (référence lente) et le compilateur (rapide) sur le même corpus |

Le fuzzing n'est pas optionnel pour un parser : c'est la différence entre un frontend qui
survit à l'utilisation réelle et un frontend qui plante au premier parenthésage étrange.

## 7. Les questions de langage (design lui-même)

Quand l'agent *conçoit* un langage, la qualité = cohérence et minimisé des surprises :
- Petite grammaire régulière au départ ; chaque feature nouvelle doit payer son coût en
  complexité de spec, d'implémentation ET de diagnostics.
- Sémantique simple à expliquer (valeurs, ownership, erreurs) avant les features avancées.
- Décisions documentées en ADR dans `knowledge/decisions.md` (syntaxe, sémantique de
  closure, système de types…) — un langage sans log de décisions devient incohérent.
- Ressource de référence : *Crafting Interpreters* (Nystrom) pour la construction
  pragmatique ; *Programming Language Pragmatics* (Scott) pour la théorie ;
  *Engineering a Compiler* (Cooper/Torczon) pour la chaîne complète.

## 8. Checklist compilateurs

- [ ] Phases séparées avec interfaces nettes ; tests par phase
- [ ] Tokens : string_view, position source, pas d'alloc par token
- [ ] Parser : descente récursive + Pratt, table de précédence, limite de profondeur
- [ ] Arena pour l'AST (lifetime homogène) ; SourceLocation sur chaque nœud
- [ ] Diagnostics centralisés, avec extrait de source ; erreurs multiples rapportées
- [ ] Panic-mode : jamais de crash sur entrée invalide
- [ ] IR séparé de l'AST ; passes d'opt testées en golden tests
- [ ] Fuzzing libFuzzer sous ASan/UBSan dans CI, corpus commité
- [ ] Round-trip + tests différentiels interpréteur/compilateur
- [ ] Décisions de design du langage loguées (ADR)
