# Domaine Rust : Ingénierie de Compilateurs, Lexers & Pratt Parsers

> Ce document établit les standards pour concevoir un compilateur ou DSL haute performance en Rust.

---

## 1. Pipeline de Compilation en 5 Étapes

```mermaid
flowchart LR
    A["Code Source (&str)"] -->|Logos| B["Tokens (Span + TokenKind)"]
    B -->|Pratt Parser| C["AST en Arène (bumpalo)"]
    C -->|Type Checker| D["Typed AST / IR"]
    D -->|Inkwell / LLVM| E["Code Machine Binaire"]
```

---

## 2. Le Lexer : Zéro Allocation avec `logos`

En Rust, l'outil de référence pour le lexing ultra-rapide est **`logos`** :
- Génère un automate fini déterministe (DFA) à la compilation via des macros de procédure.
- Opère directement sur un slice `&str` sans allouer la moindre `String`.
- Conserve les positions exactes dans le fichier (`logos::Span`).

```rust
use logos::Logos;

#[derive(Logos, Debug, PartialEq, Clone, Copy)]
pub enum TokenKind {
    #[token("fn")]
    Fn,
    #[token("let")]
    Let,
    #[regex(r"[a-zA-Z_][a-zA-Z0-9_]*")]
    Ident,
    #[regex(r"[0-9]+")]
    IntLiteral,
    #[token("+")]
    Plus,
    #[token("*")]
    Star,
    #[token("(")]
    LParen,
    #[token(")")]
    RParen,
}
```

---

## 3. Le Parseur : Algorithme de Pratt pour les Expressions

L'algorithme de Pratt (descente récursive avec priorité ascendante) est le standard mondial des compilateurs modernes (`rustc`, V8, Clang). Il élimine la récursion infinie à gauche et traite les priorités d'opérateurs en $O(N)$.

### Table de Priorités (*Binding Powers*) :
Chaque opérateur binaire possède une puissance de liaison gauche et droite :
- Addition : `(1, 2)` (associatif à gauche)
- Multiplication : `(3, 4)` (plus prioritaire que l'addition)
- Affectation : `(2, 1)` (associatif à droite)

---

## 4. L'AST en Arène avec `bumpalo`

Ne jamais allouer les nœuds d'un AST avec `Box<Node>` un par un sur le tas :
- Chaque nœud est alloué via une arène `bumpalo::Bump`.
- L'arène alloue par blocs contigus de 4 Ko / 64 Ko : le parcours de l'arbre est 3 à 5 fois plus rapide car il exploite le cache L1/L2.
- En fin de compilation, toute l'arène est libérée d'un seul coup sans parcourir les destructeurs.

---

## 5. Diagnostics Rustc-Grade avec `miette`

Les messages d'erreur doivent pointer la ligne exacte et suggérer une correction :
```rust
use miette::{Diagnostic, SourceSpan};
use thiserror::Error;

#[derive(Error, Diagnostic, Debug)]
#[error("Erreur de syntaxe : opérateur inattendu")]
#[diagnostic(code(compiler::syntax_error), help("Avez-vous oublié un opérande ?"))]
pub struct SyntaxError {
    #[label("Opérateur inattendu ici")]
    pub span: SourceSpan,
}
```

---

## 6. Checklist Actionnable pour l'Agent

- [ ] Le lexer utilise-t-il `logos` ou un DFA zéro allocation sans instancier de `String` pour les mots-clés ?
- [ ] Le parsing d'expressions utilise-t-il l'algorithme de Pratt avec des *binding powers* explicites ?
- [ ] L'AST est-il alloué dans une arène (`bumpalo`) plutôt que via des `Box` individuels dispersés ?
- [ ] Les erreurs syntaxiques incluent-elles des spans d'origine (`SourceSpan`) pour les diagnostics ?
