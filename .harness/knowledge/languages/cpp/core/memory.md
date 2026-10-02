# Mémoire, Cache & Comportement Indéfini

> **Idée directrice** : La mémoire C++ a trois étages de coût — registres → cache (L1/L2/L3)
> → RAM — et l'écart entre eux est de 2 à 3 ordres de grandeur. Le code de qualité choisit la
> *structure des données* pour le cache avant d'optimiser quoi que ce soit d'autre. Et il
> traite l'UB (undefined behavior) comme du code mort : ça compile, ça passe les tests, et ça
> explose en production.

---

## 1. La décision d'allocation : la vraie hiérarchie

| Choix | Quand | Coût |
|---|---|---|
| **Valeur / pile** (composition, `std::vector` comme membre) | Défaut | ~0 |
| Conteneurs contigus (`vector`, `array`, `string`) | Collections de taille quelconque | Amorti O(1), cache-friendly |
| `unique_ptr` | Polymorphisme, ownership transférable, PImpl | Un indirection de plus |
| `shared_ptr` | Ownership partagée réelle | Atomiques à chaque copie |
| Allocation custom (arena/pool) | Mesurée nécessaire sur hot path | À benchmarker |

**Règle** : allouer le plus tard possible, libérer le plus tôt possible — et laisser RAII faire
les deux. Préférer `reserve()` connu aux reallocs multiplicatifs sur les chemins chauds.

---

## 2. Cache & data-oriented design

### 2.1 Le modèle mental

Un cache line fait **64 octets** typiquement. Accéder à un octet charge 63 voisins. Le code
qui respecte la localité spatiale (parcours séquentiel de tableaux contigus) et temporelle
(réutilisation rapide) est 10–100× plus rapide que le code pointilliste.

```
Stroustrup : "faire un accès mémoire, c'est comme prendre un livre dans une bibliothèque
en ville. Un accès au cache L1, c'est le livre sur le bureau. L2, l'étagère.
L3/RAM, le trajet en ville."
```

### 2.2 AoS vs SoA — le choix qui compte le plus

```cpp
// ❌ Array of Structures : un parcours "utilise 8 octets, saute 24"
struct Particle { vec3 pos; vec3 vel; float mass; };   // 28 o (pad → 32)
std::vector<Particle> particles;

// ✅ Structure of Arrays : le parcours de positions est parfaitement séquentiel
struct Particles {
    std::vector<vec3> pos;
    std::vector<vec3> vel;
    std::vector<float> mass;
};
```

**Quand choisir SoA** : boucles qui ne lisent qu'un sous-ensemble des champs, traitement de
masse (SIMD, § scientific.md), gros volumes. **Quand garder AoS** : objets manipulés
individuellement, tailles modestes, code métier riche. Un programme de qualité choisit
consciemment et documente le pourquoi.

### 2.3 Branches et prédiction

Les branches mal prédites coûtent ~15 cycles. Sur les chemins chauds :
- trier/trier-partitionner les données pour rendre les branches prévisibles ;
- préférer le calcul sans branche (`std::min`, masques) quand c'est gratuit.

### 2.4 False sharing

Deux threads qui écrivent des variables **distinctes mais sur la même cache line** se
battent pour la ligne → contention invisible dans le code. Parade : padding à 64 o,
`alignas(64)`, ou privatiser les compteurs par thread puis réduire.

```cpp
struct alignas(64) PaddedCounter { std::atomic<long> value{0}; char pad[64 - sizeof(long)]; };
// sans alignas : 8 compteurs sur 2-3 cache lines = contention fantôme
```

---

## 3. Allocateurs : ne réinventer la roue que mesuré

Les faits (études Berger 2002 re-validées en 2025, arXiv 2605.17119) :

- Les allocateurs généralistes modernes (**mimalloc, jemalloc, tcmalloc**) sont excellents.
- Le seul custom allocator qui apporte un gain large et mesuré : **l'arena (région)** —
  allocations bump-pointer en O(1) (~3 cycles, proche de la pile), libération en masse.
- Les pools par-classe au-dessus de mimalloc ne gagnent ~2% — ne pas s'encombrer sauf
  preuve du contraire sur *votre* workload.

### 3.1 L'arena : quand et comment

Principe : un gros bloc, un offset, on avance ; on ne libère pas individuellement ; tout
tombe d'un coup à la destruction (ou `reset()`). Idéale pour les **lifetimes homogènes** :
tous les nœuds d'un AST (cf. compilers.md), tous les objets d'une requête, toutes les
particules d'un pas de simulation.

```cpp
class Arena {
public:
    explicit Arena(std::size_t size) : buffer_(static_cast<std::byte*>(::operator new(size))), capacity_(size) {}
    ~Arena() { ::operator delete(buffer_); }              // libération en masse (RAII)
    Arena(const Arena&) = delete; Arena& operator=(const Arena&) = delete;

    void* allocate(std::size_t size, std::size_t alignment) {
        std::size_t space = capacity_ - offset_;
        void* p = buffer_ + offset_;
        if (std::align(alignment, size, p, space) == nullptr) throw std::bad_alloc{};
        offset_ = static_cast<std::byte*>(p) - buffer_ + size;
        return p;
    }
    template <typename T, typename... Args> T* create(Args&&... args) {
        return new (allocate(sizeof(T), alignof(T))) T(std::forward<Args>(args)...);
    }
private:
    std::byte* buffer_; std::size_t capacity_, offset_ = 0;
};
```

**Points de qualité non négociables** :
- Alignement via `std::align` — jamais de bump naïf (misalignement double-coût ou crash).
- Pas thread-safe par défaut → une arena par thread, ou synchronisation documentée.
- Objets aux dtors non triviales : soit les interdire (statique), soit tenir une liste de
  destructeurs (cf. pattern protobuf Arena).
- Lier l'arena à un scope RAII. Jamais d'arena globale.

### 3.2 Règle de décision

1. Profiler d'abord (perf, valgrind, VTune) — jamais d'allocator custom "par principe".
2. Hot path confirmé → essayer mimalloc d'abord (changement de lien, zéro code).
3. Lifetime homogène identifiable → arena.
4. Toujours benchmarker avant/après et garder les deux chemins en bench de régression.

---

## 4. Undefined Behavior : la liste à connaître par cœur

UB = le compilateur peut faire n'importe quoi, y compris "ça marche chez moi". Les classes
les plus fréquentes dans le code généré par IA (mesurées : ~2× plus que le code humain) :

| UB | Exemple typique | Détecteur |
|---|---|---|
| Use-after-free / out-of-bounds | dangling après move, `v[i]` hors taille | **ASan** |
| Overflow d'entier signé | `4 * n * (n+1)` en `int` pour n grand | **UBSan** |
| Data race | lecture/écriture non synchronisées entre threads | **TSan** |
| Lecture non initialisée | `int x; if (x > 0)` | MSan / warnings |
| Violation de strict aliasing | reinterpréter `float*` en `int*` | UBSan / `-fstrict-aliasing` |
| Déréférencement de nul | `p->f()` avec `p == nullptr` | UBSan |
| Use-after-move | lire un objet moved-from | clang-tidy |
| Décalages invalides | `1 << 31`, décalage négatif | UBSan |

**Conséquences de qualité** :
- Toute écriture d'une opération arithmétique dont les opérandes peuvent déborder →
  réfléchir au type (`std::size_t`, `int64_t`) ou à `std::checked_*` (C++26 `<numeric>`,
  sinon GSL/abseil). L'overflow signé est UB ; l'overflow non signé est défini (wrap)
  mais presque toujours un bug logique.
- Le static_cast entre types de pointeurs distincts est un danger ; préférer
  `std::bit_cast` (C++20) pour la réinterprétation de bits.
- **Les sanitizers ne détectent que les chemins exécutés** : tests sérieux + fuzzing
  nécessaires (libFuzzer sur lexers/parsers, cf. compilers.md).

---

## 5. Tableau des mécanismes à privilégier

| Besoin | Privilégier | Éviter |
|---|---|---|
| Séquence + taille | `std::span<T>` (paramètres), `vector`/`array` (stockage) | `T*` seul, `T*`+`int` séparés (I.13) |
| Chaîne en paramètre (lecture) | `std::string_view` | `const std::string&` forcé, `const char*` |
| Chaîne stockée | `std::string` | char* new[]é manuel |
| Vue type-safe sur bytes | `std::byte*`, `std::bit_cast` | `char*` casts, unions type-punning |
| Placement dans un conteneur | `reserve` + `emplace_back` | push_back de temporaires en boucle |
| Accès avec contrat | `.at()` (vérifie) ou `operator[]` documenté "précondition: i < size" | `operator[]` sans précondition établie |

---

## 6. Checklist mémoire

- [ ] Structure de données choisie *pour le parcours dominant* (SoA documenté si switch)
- [ ] Pas d'allocation dans les hot loops sans mesure ; `reserve` connu utilisé
- [ ] Alignement garanti partout où on alloue soi-même (`std::align`, `alignas`)
- [ ] Pas de false sharing (compteurs par thread + réduction, padding 64 o)
- [ ] Aucun `T*` propriétaire survivant ; `unique_ptr`/valeur/composition à la place
- [ ] Aucune arithmétique signée non bornée ; types de taille choisis délibérément
- [ ] Pas de type-punning par cast de pointeur → `std::bit_cast`
- [ ] Allocation custom justifiée par un benchmark, pas par intuition
