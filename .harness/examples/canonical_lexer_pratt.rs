//! Modèle canonique Rust : lexer zéro-copie + parser de Pratt + AST en arène typée.
//!
//! Règles illustrées (corpus `.harness/knowledge/languages/rust/`) :
//! - core/ownership-borrowing.md : le code client ne manipule AUCUN pointeur brut ;
//!   l'arène possède les nœuds et prête des références immutables.
//! - core/memory.md              : arène chunkée — allocations par blocs, aucune
//!   réallocation globale, adresses stables, libération en un seul `drop`.
//! - domains/compilers.md        : lexer piloté par curseur (slices, jamais de
//!   copie), Pratt pour la précédence, golden tests avant tout.
//!
//! Production : remplacer le lexer manuel par `logos` (DFA généré à la
//! compilation) et utiliser `bumpalo` pour l'arène. Ce fichier reste
//! volontairement std-only afin d'être vérifiable tel quel :
//!   rustc --edition 2021 --test canonical_lexer_pratt.rs
//!   ./canonical_lexer_pratt        (binaire de tests : exécute les golden tests)

use std::fmt;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
enum TokKind {
    Number,
    Plus,
    Minus,
    Star,
    Slash,
    LParen,
    RParen,
    Eof,
}

#[derive(Debug, Clone, Copy)]
struct Token<'a> {
    kind: TokKind,
    lexeme: &'a str, // vue zéro-copie sur la source
}

#[derive(Debug)]
struct ParseError {
    position: usize,
    message: String,
}

impl fmt::Display for ParseError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{} à la position {}", self.message, self.position)
    }
}

impl std::error::Error for ParseError {}

// ---------------------------------------------------------------------------
// Lexer
// ---------------------------------------------------------------------------

struct Lexer<'a> {
    src: &'a [u8],
    pos: usize,
}

impl<'a> Lexer<'a> {
    fn new(src: &'a str) -> Self {
        Self { src: src.as_bytes(), pos: 0 }
    }

    fn skip_ws(&mut self) {
        while let Some(&b) = self.src.get(self.pos) {
            if b == b' ' || b == b'\t' || b == b'\n' {
                self.pos += 1;
            } else {
                break;
            }
        }
    }

    fn err(&self, message: &str) -> ParseError {
        ParseError { position: self.pos, message: message.to_owned() }
    }

    fn next_token(&mut self) -> Result<Token<'a>, ParseError> {
        self.skip_ws();
        let Some(&b) = self.src.get(self.pos) else {
            return Ok(Token { kind: TokKind::Eof, lexeme: "" });
        };
        let kind = match b {
            b'0'..=b'9' => {
                let start = self.pos;
                while matches!(self.src.get(self.pos), Some(b'0'..=b'9')) {
                    self.pos += 1;
                }
                // La grammaire n'accepte que l'ASCII : bornes UTF-8 valides par construction.
                let lexeme = std::str::from_utf8(&self.src[start..self.pos])
                    .expect("lexer ASCII : bornes valides");
                return Ok(Token { kind: TokKind::Number, lexeme });
            }
            b'+' => TokKind::Plus,
            b'-' => TokKind::Minus,
            b'*' => TokKind::Star,
            b'/' => TokKind::Slash,
            b'(' => TokKind::LParen,
            b')' => TokKind::RParen,
            _ => return Err(self.err("caractère inattendu")),
        };
        self.pos += 1;
        // Le match ci-dessus n'accepte que l'ASCII sur un octet : bornes valides.
        let lexeme = std::str::from_utf8(&self.src[self.pos - 1..self.pos])
            .expect("lexer ASCII : bornes valides");
        Ok(Token { kind, lexeme })
    }
}

// ---------------------------------------------------------------------------
// Arène typée chunkée : adresses stables, libération en un drop, ids stables.
// (En production : bumpalo — même discipline, sans unsafe.)
// ---------------------------------------------------------------------------

struct Arena<T> {
    chunks: Vec<Vec<T>>,
}

impl<T> Arena<T> {
    const CHUNK: usize = 256;

    fn new() -> Self {
        Self { chunks: Vec::new() }
    }

    fn alloc(&mut self, value: T) -> usize {
        if self.chunks.last().map_or(true, |c| c.len() == c.capacity()) {
            self.chunks.push(Vec::with_capacity(Self::CHUNK));
        }
        let chunk_idx = self.chunks.len() - 1;
        let chunk = self.chunks.last_mut().expect("chunk fraîchement poussé");
        chunk.push(value);
        let item_idx = chunk.len() - 1;
        chunk_idx * Self::CHUNK + item_idx
    }

    fn get(&self, id: usize) -> &T {
        &self.chunks[id / Self::CHUNK][id % Self::CHUNK]
    }
}

// ---------------------------------------------------------------------------
// AST : indexé (pas de lifetimes en pagaille, cache-friendly)
// ---------------------------------------------------------------------------

#[derive(Debug, Clone, Copy)]
enum BinOp {
    Add,
    Sub,
    Mul,
    Div,
}

#[derive(Debug, Clone, Copy)]
enum Expr {
    Number(f64),
    Binary { op: BinOp, lhs: usize, rhs: usize },
}

// ---------------------------------------------------------------------------
// Parser de Pratt
// ---------------------------------------------------------------------------

struct Parser<'a, 'arena> {
    lexer: Lexer<'a>,
    current: Token<'a>,
    arena: &'arena mut Arena<Expr>,
}

impl<'a, 'arena> Parser<'a, 'arena> {
    fn new(src: &'a str, arena: &'arena mut Arena<Expr>) -> Result<Self, ParseError> {
        let mut lexer = Lexer::new(src);
        let current = lexer.next_token()?;
        Ok(Self { lexer, current, arena })
    }

    fn advance(&mut self) -> Result<(), ParseError> {
        self.current = self.lexer.next_token()?;
        Ok(())
    }

    fn parse_expression(&mut self, min_precedence: i32) -> Result<usize, ParseError> {
        let mut lhs = self.parse_primary()?;
        loop {
            let precedence = Self::precedence_of(self.current.kind);
            if precedence < min_precedence {
                break;
            }
            let op_kind = self.current.kind;
            self.advance()?;
            // + 1 => associativité gauche ("8 / 4 / 2" == (8/4)/2).
            let rhs = self.parse_expression(precedence + 1)?;
            let op = Self::to_binop(op_kind);
            lhs = self.arena.alloc(Expr::Binary { op, lhs, rhs });
        }
        Ok(lhs)
    }

    fn parse_primary(&mut self) -> Result<usize, ParseError> {
        match self.current.kind {
            TokKind::Number => {
                let value: f64 = self
                    .current
                    .lexeme
                    .parse()
                    .map_err(|_| self.lexer.err("nombre invalide"))?;
                self.advance()?;
                Ok(self.arena.alloc(Expr::Number(value)))
            }
            TokKind::LParen => {
                self.advance()?;
                let inner = self.parse_expression(0)?;
                if self.current.kind != TokKind::RParen {
                    return Err(self.lexer.err("')' attendu"));
                }
                self.advance()?;
                Ok(inner)
            }
            _ => Err(self.lexer.err("opérande attendu")),
        }
    }

    const fn precedence_of(kind: TokKind) -> i32 {
        match kind {
            TokKind::Plus | TokKind::Minus => 1,
            TokKind::Star | TokKind::Slash => 2,
            _ => -1,
        }
    }

    fn to_binop(kind: TokKind) -> BinOp {
        match kind {
            TokKind::Plus => BinOp::Add,
            TokKind::Minus => BinOp::Sub,
            TokKind::Star => BinOp::Mul,
            TokKind::Slash => BinOp::Div,
            _ => unreachable!("to_binop n'est appelé que sur un opérateur"),
        }
    }
}

fn evaluate(arena: &Arena<Expr>, id: usize) -> f64 {
    match arena.get(id) {
        Expr::Number(v) => *v,
        Expr::Binary { op, lhs, rhs } => {
            let l = evaluate(arena, *lhs);
            let r = evaluate(arena, *rhs);
            match op {
                BinOp::Add => l + r,
                BinOp::Sub => l - r,
                BinOp::Mul => l * r,
                BinOp::Div => {
                    assert!(r != 0.0, "division par zéro évaluée");
                    l / r
                }
            }
        }
    }
}

fn eval(src: &str) -> Result<f64, ParseError> {
    let mut arena = Arena::new();
    let root = Parser::new(src, &mut arena)?.parse_expression(0)?;
    Ok(evaluate(&arena, root))
}

fn main() {
    assert_eq!(eval("1 + 2 * 3").unwrap(), 7.0);
    assert_eq!(eval("(1 + 2) * 3").unwrap(), 9.0);
    assert_eq!(eval("8 / 4 / 2").unwrap(), 1.0);
    assert!(eval("1 +").is_err());
    println!("Modèle Pratt + arène Rust validé (golden tests + erreur positionnée).");
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn precedence_pratt() {
        assert_eq!(eval("1 + 2 * 3").unwrap(), 7.0);
    }

    #[test]
    fn parentheses() {
        assert_eq!(eval("(1 + 2) * 3").unwrap(), 9.0);
    }

    #[test]
    fn associativite_gauche() {
        assert_eq!(eval("10 - 2 - 3").unwrap(), 5.0);
    }

    #[test]
    fn erreur_positionnee() {
        let err = eval("1 +").unwrap_err();
        assert!(err.to_string().contains("position"));
    }
}
