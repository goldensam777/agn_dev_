# Gestion des Erreurs

> **Idée directrice** : C++ n'impose pas de style de gestion d'erreurs — il faut donc en
> **choisir un et l'appliquer partout**. Le style recommandé par les Core Guidelines et la
> pratique moderne : exceptions pour les erreurs qui doivent remonter, `std::expected`
> (C++23) pour les erreurs attendues au niveau des frontières, assert pour les bugs.
> Une erreur doit remonter **au premier niveau capable d'agir — pas plus haut**.

---

## 1. Le triptyque : assert / expected / exceptions

| Mécanisme | Signifie | Exemple |
|---|---|---|
| `assert(condition)` | "C'est un **bug** si c'est faux. Le code est correct par hypothèse." | invariant interne, état impensable |
| `std::expected<T, E>` (C++23) ou `std::optional<T>` | "L'échec est un **résultat normal**, l'appelant choisit." | parsing, lookup, validation utilisateur |
| `throw` | "Échec d'une tâche requise, l'appelant éloigné doit traiter." | fichier illisible, allocation impossible, contrat d'API public violé |

### Règles d'usage

**Assertions** — pour les erreurs de *programmation*, pas d'environnement :
```cpp
double fastInvSqrt(double x) {
    assert(x > 0.0);            // bug si appelé avec x ≤ 0 : la faute est au caller
    // ...
}
```
- Jamais d'effet de bord dans un assert (peut être compilé hors en NDEBUG).
- En release, les invariants critiques restent des vérifications réelles, pas des asserts.

**Exceptions** — quand détecteur et gestionnaire sont séparés par des couches :
```cpp
// détection (3 couches sous le gestionnaire)
void writeChunk(std::ostream& os, const Data& d) {
    if (!os) throw std::ios_base::failure{"writeChunk: stream not writable"};
    os.write(d.bytes(), d.size());
}
// gestionnaire (le seul endroit qui sait quoi faire)
void saveDocument(const Document& doc, const Path& p) {
    try {
        auto os = openOutput(p);
        writeChunk(os, doc.header());
        writeChunk(os, doc.body());
    } catch (const std::ios_base::failure& e) {
        showError(tr("Could not save: %1").arg(e.what()));   // retry ? annuler ? message ?
    }
}
```
Les fonctions intermédiaires ne font **rien** : c'est la force du mécanisme.

**std::expected** — pour les échecs *attendus et locaux*, surtout aux frontières de bibliothèque
et où la performance du chemin d'erreur compte (pas de stack unwinding) :
```cpp
std::expected<Config, ParseError> loadConfig(std::string_view path);
// appelant : erreur = donnée, pas exception
auto cfg = loadConfig(path);
if (!cfg) return std::unexpected(cfg.error().withContext(path));
```

### Quand PAS d'exceptions
- Boucle de calcul très chaude où l'erreur est gérée au même niveau → code d'erreur/expected
  (le coût du throw est payé seulement en cas d'erreur, mais la *possibilité* a un coût
  structurel : chaque fonction doit être exception-safe).
- Frontières `noexcept` : destructeurs, fonctions C callbacks, certains move.
- Le projet interdit les exceptions (embedded, certaines bases de code) → alors expected
  partout, de façon *systématique* (style Rust-like), jamais de mélange à la demande.

---

## 2. Les garanties d'exception-safety

Toute fonction qui peut échouer (directement ou en appelant) offre l'une de ces garanties.
**La déclarer et la tenir est un critère de qualité** (Sutter, Exceptional C++) :

| Garantie | Promesse | Comment |
|---|---|---|
| **Nothrow** (`noexcept`) | Ne throw jamais | Tout est no-fail (swaps, dtors, move) |
| **Forte** | Échec ⇒ état exactement comme avant | commit-or-rollback : construire dans des temporaux, ne muter qu'au dernier moment via swap |
| **De base** | Échec ⇒ pas de fuite, pas de structure corrompue, mais état modifié possible | RAII sur toutes les ressources |

Le RAII est le préalable de tout : sans lui, même la garantie de base est un miracle.
```cpp
// Forte garantie : la mutation n'est publiée que si tout a réussi
void Container::addAll(const std::vector<Item>& items) {
    auto tmp = data_;                    // copie (ou clone COW)
    tmp.insert(tmp.end(), items.begin(), items.end());   // peut throw : tmp seul est touché
    data_.swap(tmp);                     // no-fail : commit
}
```

**Règles de forme** :
- Throw **par valeur**, catch **par référence const** (`catch (const E& e)`).
- Ne jamais laisser d'exception s'échapper d'un destructeur → dtors implicitement `noexcept`.
- Pas de spécifications d'exceptions `throw(...)` (dépréciées) ; `noexcept` est le seul
  contrat de non-throw.
- `catch (...)` seulement en dernier recours + rethrow ou log structuré — ne jamais
  avaler silencieusement.
- "Don't catch what you can't handle" : attraper pour traiter, sinon laisser passer.

---

## 3. Concevoir la politique d'erreurs par couche

Une architecture de qualité fixe *qui traite quoi où* — sinon chaque fonction invente sa
politique et les erreurs fuient dans tous les sens :

```
┌─────────────────────────────────────────────────┐
│ UI / API publique        ← messages, retry, I18N │  (exceptions attrapées ici)
├─────────────────────────────────────────────────┤
│ Services / domaine       ← transformer, enrichir │  (envelopper l'erreur de contexte)
├─────────────────────────────────────────────────┤
│ Bibliothèques            ← throw / expected      │  (contrats précis, jamais de cout)
├─────────────────────────────────────────────────┤
│ Kernels hot path         ← codes d'erreur locaux │  (jamais de throw dans la boucle)
└─────────────────────────────────────────────────┘
```

Exemples de règles concrètes :
- **Serveur web** : la plupart des erreurs remontent au niveau requête → 5xx + log, kill de
  la requête. Une panne réseau globale ne doit pas être redécouverte par chaque requête.
- **Bibliothèque** : jamais de `std::exit`, jamais de `cout/cerr`, jamais de traitement
  silencieux — laisser la décision à l'appelant.
- **Fonctions internes** : préconditions vérifiées par assert, postconditions documentées.

---

## 4. Les erreurs ont besoin de contexte

Une exception sans contexte coûte des heures de debug. Les exceptions custom doivent porter
les données du diagnostic :

```cpp
struct ParseError : std::runtime_error {
    ParseError(std::string_view msg, SourceLocation loc)   // ligne, colonne, extrait
        : std::runtime_error(std::string{loc.file} + ":" + std::to_string(loc.line)
                             + ": " + std::string{msg}),
          location(loc) {}
    SourceLocation location;
};
```

- Hiérarchie d'exceptions du projet : hériter de `std::exception` (ou `std::runtime_error`,
  `std::logic_error`, `std::invalid_argument`, `std::out_of_range` quand la sémantique
  standard correspond — I.10, ne pas réinventer).
- `std::source_location` (C++20) pour le point d'origine.
- Consigner l'erreur **une fois**, là où elle est traitée — pas dans chaque couche traversée
  (sinon : 5 logs pour une erreur, et personne ne sait lequel compte).

---

## 5. Anti-patterns (interdits)

| ❌ Anti-pattern | Pourquoi | ✅ À la place |
|---|---|---|
| `catch (...) {}` silencieux | l'erreur disparaît, l'état devient incohérent | traiter ou laisser remonter |
| Code d'erreur C global (`errno`-style) | non typé, pas de portée, oubli facile | expected / exceptions |
| Retour magique `-1` / `nullptr` sans doc | convention implicite, oubli du test | `std::optional`, `expected`, exception |
| throw `const char*` | rien à attraper précisément | types dérivant de `std::exception` |
| `is_valid()` + init en deux phases | état zombie dans le type | ctor qui throw, ou factory |
| Exception dans la signature de la hot loop | perf + raisonnement impossibles | code d'erreur local, vérif hors boucle |
| Log-and-rethrow partout | bruit, double comptage, contexte noyé | log une fois au point de traitement |
| Assert avec effet de bord | disparait en NDEBUG, bug silencieux | assert pur sur une variable déjà calculée |

---

## 6. Checklist erreurs

- [ ] Politique par couche explicitée (où se traite chaque catégorie d'erreur)
- [ ] assert = bug du programme ; exception = échec de tâche ; expected = résultat normal
- [ ] Garantie de chaque fonction raisonnée (nothrow/forte/base) et tenue via RAII
- [ ] throw par valeur, catch par `const&`, dtors noexcept
- [ ] Aucune exception ne traverse une frontière C (`extern "C"`, callbacks)
- [ ] Exceptions custom = contexte riche (location, données), héritage `std::exception`
- [ ] Aucun `catch (...)` avalant ; aucun log-and-rethrow redondant
- [ ] Chemin d'erreur testé (test qui force l'échec — sinon la garantie n'est pas démontrée)
