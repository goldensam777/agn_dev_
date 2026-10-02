# Core Rust : Conception Guidée par les Types & Traits

> Ce document détaille l'utilisation du système de types de Rust comme outil de preuve formelle à la compilation.

---

## 1. Le Pattern "Newtype" : Éliminer la Confusion Primitives

Ne jamais passer des types primitifs nus (`u64`, `f64`, `String`) lorsque la sémantique est distincte :
```rust
// ❌ MAUVAIS (Risque d'inversion des paramètres à l'appel)
fn transfer(sender_id: u64, receiver_id: u64, amount: f64) { ... }

// ✓ BON (Impossible d'inverser par accident)
#[repr(transparent)]
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub struct UserId(pub u64);

#[repr(transparent)]
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct AmountUSD(pub f64);

fn transfer(sender: UserId, receiver: UserId, amount: AmountUSD) { ... }
```
*Note : `#[repr(transparent)]` garantit un coût mémoire et runtime strictement nul ($0$ overhead).*

---

## 2. Le Pattern "Typestate" : États Invalides Inexpressibles

Remplacer les drapeaux booléens (`is_authenticated`, `is_validated`) par des types distincts à la compilation :
```rust
pub struct Draft;
pub struct Verified;
pub struct Published;

pub struct Document<State> {
    title: String,
    content: String,
    _state: std::marker::PhantomData<State>,
}

impl Document<Draft> {
    pub fn new(title: String, content: String) -> Self { ... }
    pub fn verify(self) -> Document<Verified> { ... }
}

impl Document<Verified> {
    pub fn publish(self) -> Document<Published> { ... }
}

// Impossible d'appeler publish() sur un Document<Draft> : erreur de compilation !
```

---

## 3. Dispatch Statique (`impl Trait`) vs Dynamique (`dyn Trait`)

| Critère | Dispatch Statique (`impl Trait` / `<T: Trait>`) | Dispatch Dynamique (`&dyn Trait` / `Box<dyn Trait>`) |
|---|---|---|
| **Mécanisme** | Monomorphisation à la compilation (spécialisation de code). | Pointeur de table virtuelle (*vtable* / fat pointer). |
| **Inlining & SIMD** | Totalement possible, optimisations LLVM maximales. | Impossible (saut indirect de pointeur de fonction). |
| **Taille binaire** | Risque d'inflation (*code bloat*) si trop de variantes. | Très compacte (une seule instance de code machine). |
| **Hétérogénéité** | Impossible dans une même collection (`Vec<T>` homogène). | Parfait pour `Vec<Box<dyn Node>>`. |

**Règle d'or :**
- Utiliser le dispatch statique dans les boucles de calcul intensif, les parseurs et les algorithmes numériques.
- Réserver le dispatch dynamique aux frontières de plugins, aux collecteurs hétérogènes ou lorsque la monomorphisation ralentit excessivement les temps de compilation.

---

## 4. Checklist Actionnable pour l'Agent

- [ ] Les identifiants ou unités physiques utilisent-ils le pattern *newtype* avec `#[repr(transparent)]` ?
- [ ] Les machines à états utilisent-elles le pattern *typestate* plutôt que des mutabilités avec `Option` ou drapeaux booléens ?
- [ ] Tout trait standard pertinent (`Debug`, `Default`, `Display`, `PartialEq`) est-il dérivé ou implémenté ?
- [ ] Le choix entre `impl Trait` et `dyn Trait` est-il justifié par la performance ou le polymorphisme ?
