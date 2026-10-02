# C++ Moderne (C++17/20/23) & Idiomes de Conception

> **Idée directrice** : Chaque standard récent existe pour supprimer une classe de bugs ou
> une classe de lourdeur. Le C++ de qualité *aujourd'hui* n'est pas le C++ de 2011 écrit
> plus tard : c'est du code où les features modernes portent les invariants. Mais un feature
> nouveau n'est pas une raison de réécrire du code qui marche (KISS) — il est une raison
> d'écrire *le nouveau code* mieux.

---

## 1. Les basiques C++17 (déjà non négociables)

```cpp
// if/switch avec initialiseur : la variable vit exactement au bon scope
if (auto it = cache.find(key); it != cache.end()) return it->second;

// structured bindings : déstructurer sans commentaire
auto [mean, var] = stats(data);

// string_view : paramètre "chaîne en lecture" sans copier, sans imposer std::string
void log(std::string_view msg);

// optional / variant : nullabilité et sommes dans le type system
std::optional<Config> findConfig(std::string_view name);

// CTAD : std::lock_guard lock{mtx}; std::pair p{1, 2.5};
```

## 2. C++20 — les quatre grosses

### Concepts : des contrats de template lisibles (et des erreurs de compilation humaines)

```cpp
// ❌ erreur à l'intérieur du corps, 300 lignes de template noise
template <typename T> double norm(const T& v) { return std::sqrt(v.x*v.x + v.y*v.y); }

// ✅ le contrat est dans la signature ; l'erreur est au point d'appel, claire
template <typename V>
concept Vec2 = requires(const V& v) { { v.x } -> std::convertible_to<double>;
                                     { v.y } -> std::convertible_to<double>; };

template <Vec2 V> double norm(const V& v) { return std::hypot(v.x, v.y); }
```

### Ranges : décrire le *quoi*, pas le *comment*

```cpp
// pipeline déclaratif, lazy, sans index ni tailles manuelles
auto active_names = users
    | std::views::filter(&User::isActive)
    | std::views::transform(&User::name)
    | std::views::take(10);
```

### `constexpr` généralisé / `consteval`

- F.4 : déclarer `constexpr` dès que possible — ça élargit l'usage (compile-time et run-time)
  sans rien coûter.
- `consteval` : fonction *forcée* à l'évaluation à la compilation (l'anti-fuite quand la
  compile-time evaluation est une exigence).
- De plus en plus de la bibliothèque standard est `constexpr` : `std::vector` (allocation
  constexpr C++20), algorithmes, `std::complex`… — les calculs qui peuvent être faits à la
  compilation *doivent* l'être.

### `<=>` et les comparaisons par défaut

```cpp
struct Version {
    int major, minor, patch;
    auto operator<=>(const Version&) const = default;   // ==, !=, <, <=, >, >= : cohérents
};
```

## 3. C++20/23 — le quotidien qui s'améliore

| Feature | Usage de qualité |
|---|---|
| `std::span` | Tout paramètre "tableau + taille" → `std::span<T>` (bounds, contiguïté, auto-vectorisation) |
| `std::format` | Remplacer iostreams pour le formatage texte (typesafe, rapide, lisible) |
| `std::expected` | Frontières d'erreur (cf. errors.md) |
| `std::jthread` + `stop_token` | Threads qui s'arrêtent proprement : la cancellation devient un contrat |
| `std::source_location` | Origine des diagnostics sans macros `__LINE__` |
| `std::bit_cast` | Réinterprétation de bits, propre et sans UB |
| `std::to_underlying`, `std::byteswap` | Boilerplate d'enums et d'endianness éliminé |
| `if consteval` | Un seul code pour les deux mondes (compile-time/run-time) |
| `import std;` (modules) | Adopter progressivement ; l'écosystème build reste maturant — ne pas migrer un projet entier pour le principe |

## 4. Idiomes de conception (le vocabulaire du code senior)

### 4.1 Type erasure — polymorphisme sans héritage
Quand "différents types, même comportement" ne justifie pas une hiérarchie :
`std::function`, `std::any`, ou l'erasure manuelle (`unique_ptr<Concept>`, vtable maison).
Plus cher qu'un virtuel, mais découplé.

### 4.2 CRTP — héritage statique
Polymorphisme à la compilation (zéro indirection), pour les mixins et le static dispatch :
```cpp
template <typename Derived> struct Comparable {
    friend bool operator==(const Comparable& a, const Comparable& b) {
        return static_cast<const Derived&>(a).key() == static_cast<const Derived&>(b).key();
    }
};
class User : public Comparable<User> { /* … */ };
```

### 4.3 PImpl — compilation firewall
`class X { struct Impl; std::unique_ptr<Impl> pImpl_; };` dans le header, définition de
`Impl` dans le .cpp. Stabilité d'ABI, temps de compilation, headers propres.

### 4.4 Copy-and-swap
Pour l'assignation forte-garantie avec un seul point de mutation :
```cpp
Matrix& operator=(Matrix other) noexcept { swap(*this, other); return *this; }
// pass-by-value absorbe la copie OU le move selon l'appelant
```

### 4.5 Scope guard / RAII généralisé
Pour toute action acquire/release non couverte par un type standard : un petit objet
dont le dtor fait le cleanup (rollback de transaction, restore de flag, etc.).

### 4.6 NVI (Non-Virtual Interface)
La fonction publique non-virtuelle vérifie les invariants et appelle le hook virtuel privé
— le contrat est non-virtual, la customisation est privée.

### 4.7 Policy-based design
Le comportement variant devient un paramètre de template (stratégie à la compilation).
Puissant, mais à doser : chaque instantiation est un type distinct — mesurer le coût
binaire avant d'en abuser.

## 5. Ce que le code moderne *évite* (à reconnaître en review)

| Vieux réflexe | Moderne |
|---|---|
| `NULL` / `0` pour le pointeur nul | `nullptr` |
| `typedef` | `using` |
| `std::endl` (flush à chaque ligne !) | `'\n'` ; flush explicite seulement |
| `rand()` / `srand()` | `<random>` : `mt19937`, distributions typées |
| `throw()` | `noexcept` |
| `auto_ptr` | `unique_ptr` |
| `boost::optional` (C++11) | `std::optional` |
| Macros de constantes | `constexpr` |
| `sprintf`, `strcpy`, format C | `std::format`, `snprintf`, spans |
| `for (auto i = 0u; …)` et comparaisons signé/non signé mêlées | `std::size_t` cohérent, `-Wsign-conversion` |
| Boucles indexées brutes (quand l'algorithme dit l'intention) | algorithms/ranges : `find_if`, `accumulate`, `transform` |

## 6. Checklist C++ moderne

- [ ] Concepts sur les templates génériques publics ; pas de template nu non documenté
- [ ] `span` pour les séquences en paramètre ; `string_view` pour les lectures de chaînes
- [ ] `constexpr`/`consteval` dès que la sémantique le permet
- [ ] Comparaisons par défaut (`<=>`) ; `enum class` ; types forts aux frontières
- [ ] `<random>` pour toute aléatoire ; distributions explicites
- [ ] Idiomes choisis consciemment (erasure/CRTP/PImpl) avec leur coût connu
- [ ] Pas de réécriture cosmétique de code legacy fonctionnel (KISS)
- [ ] Features nécessitant un compiler récent documentées dans CONVENTIONS.md
