# Ownership & RAII — Le socle du C++ de qualité

> **Idée directrice** : En C++, l'ownership n'est pas une convention de documentation —
> c'est une **décision de design qui doit être visible dans les types**. Un lecteur (humain ou
> agent) doit pouvoir dire qui détruit quoi sans lire le corps des fonctions.
>
> Source de référence : C++ Core Guidelines (isocpp.github.io), sections P.8, R.20–R.37, C.32–C.40, I.11, F.7.

---

## 1. Le principe RAII

**Resource Acquisition Is Initialization** : une ressource (mémoire, fichier, socket, mutex,
transaction DB…) est acquise dans le constructeur d'un objet et **relâchée dans son destructeur**.
Le destructeur s'exécute *toujours* — succès, early return, exception — grâce au stack unwinding.

```cpp
// ❌ Mauvais : une ressource, N chemins de sortie, N chances de fuite
void f(const char* name) {
    FILE* input = fopen(name, "r");
    if (something) return;          // fuite si something
    // ...
    fclose(input);                  // oubli facile, pas exception-safe
}

// ✅ Bon : la ressource est un objet, la libération est garantie
void f(const std::string& name) {
    std::ifstream input{name};      // ouverture = construction
    if (something) return;          // OK : ~ifstream() ferme
    // ...
}                                   // fermeture garantie sur tous les chemins
```

**Conséquence directe pour le reviewer** : si tu vois `new`/`delete`, `fopen`/`fclose`,
`lock`/`unlock` appariés manuellement dans du code nouveau, c'est un défaut. Sauf cas très
justifié (interfaçage C, hardware), remplacer par un handle RAII.

### La déclinaison du RAII : scope guards

Toute action "acquérir puis relâcher" doit être un objet. Exemple mutex (Core Guideline I.7) :

```cpp
// ❌ postcondition implicite et non garantie : "m est déverrouillé"
void manipulate(Record& r) {
    m.lock();
    if (bad) return;                // deadlock
    // ...
    m.unlock();
}

// ✅ le lock_guard EST la postcondition
void manipulate(Record& r) {
    std::lock_guard<std::mutex> lock{m};
    if (bad) return;                // OK
    // ...
}
```

---

## 2. La hiérarchie d'ownership : qui possède quoi

| Mécanisme | Sémantique | Coût | Quand |
|---|---|---|---|
| Objet par valeur / composition | Propriété exclusive, inline | Zéro | **Défaut absolu** |
| `std::unique_ptr<T>` | Propriété exclusive, pointée | ~Coût d'un raw pointer | Polymorphisme, ownership transférable |
| `std::shared_ptr<T>` | Propriété partagée (comptage) | Atomique à chaque copie | Vraie propriété partagée, surtout cross-thread |
| `std::weak_ptr<T>` | Observation non-propriétaire | Faible | Casser les cycles, observer un shared |
| `T*` / `T&` | **Non-propriétaire**, observation | Zéro | Paramètres, accès internes, optionnalité (`T*`) |

**Règles d'or (Core Guidelines R.20–R.24, R.21 en tête)** :

1. **Règle n°1 : `unique_ptr` par défaut, `shared_ptr` seulement si l'ownership est réellement
   partagée.** Le `shared_ptr` a un coût réel (opérations atomiques sur le compteur à chaque
   copie/incrément) et il masque la structure de lifetime du programme. Si un seul propriétaire
   suffit, `unique_ptr`. Si tu hésites entre les deux, c'est `unique_ptr`.
2. **`make_unique` / `make_shared` systématiquement** (exception : constructeur privé,
   contrôle d'allocation custom, `weak_ptr` longue durée avec `make_shared` → le bloc compteur
   reste alloué tant qu'un `weak_ptr` vit).
3. **Jamais de transfert d'ownership par raw pointer ou référence** (I.11) :
   ```cpp
   X* compute(args);            // ❌ qui delete ? ambiguïté = fuite ou double-free
   std::unique_ptr<X> compute(args);   // ✅ la signature dit tout
   ```
4. **Un pointeur brut est non-propriétaire par défaut.** C'est le point de vue du compilateur,
   des analyseurs statiques et du lecteur. Si un membre `T*` possède, c'est un défaut de design
   (C.32) : soit composition, soit `unique_ptr`, soit le marquer `owner<T*>` (GSL) en code legacy.
5. **Cycles de `shared_ptr` = fuite.** Un cycle ne retombe jamais à zéro. Casser avec un
   `weak_ptr` du côté "faible" de la relation (enfant → parent, cache → élément).
6. **Références : jamais propriétaires, jamais nulles, jamais rebindables** (R.4). Besoin de
   null ou de rebind ? `T*` ou `std::optional<std::reference_wrapper<T>>`.

---

## 3. Règle des 0 / 3 / 5 (Rule of Zero)

**Règle des 0 (à viser)** : une classe qui n'a pas de ressource gérée manuellement ne doit
déclarer **aucune** opération spéciale. Le compilateur génère tout, correctement. Les types
avec `unique_ptr`, `string`, `vector` comme membres suivent déjà cette règle — *ne déclare pas
de destructeur "pour être explicite"*.

**Règle des 5** : si tu déclares *une seule* de ces opérations, tu dois raisonner sur (et
généralement déclarer ou `= default` / `= delete`) les cinq :

```cpp
class Buffer {
public:
    Buffer(std::size_t n);                        // ctor
    ~Buffer();                                    // dtor  → oblige à penser aux 4 autres
    Buffer(const Buffer&);                        // copy ctor
    Buffer& operator=(const Buffer&);             // copy assign
    Buffer(Buffer&&) noexcept;                    // move ctor
    Buffer& operator=(Buffer&&) noexcept;         // move assign
};
```

Le cas classique du bug (C.33) : un destructeur qui `delete` un membre `T*` + opérations de
copie par défaut = double libération silencieuse. **Le correctif n'est pas d'écrire les 5
opérations à la main, c'est de remplacer le membre `T*` par `unique_ptr`** → règle des 0.

**Les move doivent être `noexcept`** : sinon `std::vector` refuse de move lors du realloc
(préfère copier) et les exceptions rendent les opérations ambiguës.

**État moved-from** : un objet après `std::move` est dans un état valide mais non spécifié.
Seules les opérations sans précondition dessus sont sûres (destruction, réassignation).
*Utiliser* un objet moved-from (lire sa valeur) est un bug que clang-tidy signale
(`bugprone-use-after-move`).

```cpp
auto a = std::make_unique<Foo>();
auto b = std::move(a);
use(*a);   // ❌ a est vide : UB. clang-tidy le voit, un humain parfois pas.
```

---

## 4. Passage de paramètres : la table de décision

(Core Guidelines F.15–F.21 — les techniques "avancées" ne se justifient que mesurées et
commentées.)

| Intention | Signature | Exemple |
|---|---|---|
| Entrée, type petit (≤ 2 pointeurs) | par valeur | `int multiply(int a, int b)` |
| Entrée, type coûteux à copier | `const T&` | `void log(const Record& r)` |
| Entrée, "je consomme/vole" | `T` + `std::move` côté corps | `void store(std::string s)` |
| Sortie unique | **retour par valeur** | `std::vector<double> compute()` |
| Plusieurs sorties | retourner un struct | `struct Stats { double mean, var; };` |
| Entrée-sortie (modifié) | `T&` | `void normalize(Vec& v)` |
| Optionnalité (peut être absent) | `T*` (ou `std::optional<T&>` non standard) | `void setParent(Node* p)` |
| **Transfert d'ownership** | `std::unique_ptr<T>` | `void adopt(std::unique_ptr<Pet> p)` |
| **Reseat possible du smart ptr** | `std::unique_ptr<T>&` | `void resetTo(std::unique_ptr<T>& p, ...)` |

**Anti-règles** :
- Ne prends pas `const shared_ptr<T>&` juste pour *lire* l'objet → prends `const T&` ou `T*`
  (F.7 : une fonction qui ne manipule pas la lifetime ne doit pas imposer un type de smart
  pointer à l'appelant ; ça interdit les objets stack !).
- Ne retourne jamais un `T*` fraîchement alloué pour transférer la propriété (F.26, I.11).
- Un `T*` retourné indique une **position** dans une structure appartenant à l'appelant
  (F.42), jamais une ressource nouvelle.

---

## 5. Constructeurs et invariants

- **Un constructeur doit produire un objet valide ou thrower** (C.42). Pas d'idiome
  `init()` / `is_valid()` en deux phases : délégation de constructeurs + initialisation de
  membres par défaut font le travail proprement. Exception : hard real-time sans exceptions.
- **Le destructeur ne throw jamais** (C.36) : pendant le stack unwinding, un throw dans un
  dtor → `std::terminate`.
- Établis les **invariants de classe** explicitement et fais-les respecter par le type
  (types plus précis, `not_null`, enums) plutôt que par des commentaires.

```cpp
class Grid {
public:
    Grid(std::size_t nx, std::size_t ny)
        : nx_(nx), ny_(ny), data_(nx * ny)   // invariant : data_.size() == nx_*ny_, toujours
    {
        if (nx == 0 || ny == 0) throw std::invalid_argument("Grid: dimensions must be > 0");
    }
    double& at(std::size_t i, std::size_t j);   // bounds-checked, documente le contrat
private:
    std::size_t nx_, ny_;
    std::vector<double> data_;
};
```

---

## 6. Checklist rapide (à charger dans le contexte du Builder/Reviewer)

- [ ] Aucun `new`/`delete` nu, aucun appariement manuel de ressources (RAII partout)
- [ ] Ownership visible dans les types : valeur → `unique_ptr` → `shared_ptr` (rare) → `weak_ptr` (cycles)
- [ ] `make_unique`/`make_shared` ; jamais d'ownership via `T*`/`T&` dans une API
- [ ] Rule of Zero respectée ; si opération spéciale → les 5, move `noexcept`
- [ ] Pas de `shared_ptr` "par confort" ; justifié + documenté sinon
- [ ] Rien n'utilise d'objet after-move sauf destruction/réassignation
- [ ] Paramètres : petits par valeur, gros `const&`, sorties par valeur, in-out `T&`
- [ ] Constructeurs valides-ou-throw ; dtors `noexcept` implicite
- [ ] Invariants de classe encodés dans les types, pas dans des commentaires
