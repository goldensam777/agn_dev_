# Anti-Patterns C++ — Interdits et Leurs Remèdes

> **Mode d'emploi pour l'agent** : ce fichier est la liste rouge. En écriture : ne jamais
> en produire un. En review : chaque occurrence = demande de correction, sauf exception
> documentée dans le code (`// rationale: …`) et acceptée par le reviewer.

| # | Anti-pattern | Pourquoi c'est mauvais | Le remède |
|---|---|---|---|
| 1 | `new`/`delete` nu, `malloc`/`free` appariés à la main | fuites, double-free, exception-unsafe | RAII, `make_unique`/`make_shared`, handles |
| 2 | `shared_ptr` "par défaut" | atomiques partout, cycles masqués, ownership illisible | `unique_ptr`/valeur ; `shared_ptr` justifié |
| 3 | Membre `T*` propriétaire | double-free à la copie (règle des 5 oubliée) | composition, `unique_ptr`, ou `owner<T*>` (legacy) |
| 4 | Ownership transféré via `T*`/`T&` retourné (I.11) | "qui delete ?" → fuite ou crash | retour par valeur (move) ou `unique_ptr` |
| 5 | Variable globale mutable / singleton | dépendances cachées, races, tests impossibles | injection de dépendances, constantes globales ok |
| 6 | `using namespace std;` (surtout dans un header) | pollution, collisions silencieuses | qualifications explicites |
| 7 | C-style cast `(int)x` | n'importe lequel des 4 casts, peut casser const | `static_cast`/`const_cast` justifié/`reinterpret_cast` jamais à l'aveugle ; `bit_cast` pour les bits |
| 8 | `catch (...) {}` ou `catch(const std::exception&){}` vide | l'erreur disparaît, état incohérent | traiter, ou ne pas attraper |
| 9 | `throw "message"` / types hors `std::exception` | rien à attraper proprement | hiérarchie dérivant `std::exception` |
| 10 | Initialisation en deux phases (`init()` + `is_valid()`) | objet zombie dans le type | ctor qui throw / factory |
| 11 | Dtor qui throw | `std::terminate` pendant unwinding | dtor `noexcept`, cleanup no-fail |
| 12 | `rand()`/`srand()` | qualité statistique pauvre, non reproductible proprement | `<random>` + seed explicite |
| 13 | `std::endl` en boucle ou sur le chemin de log | flush système à chaque appel | `'\n'` ; flush explicite |
| 14 | Comparaison `==` de flottants calculés | faux dès que les opérations diffèrent | tolérance motivée, reformulation (cf. scientific.md) |
| 15 | Boucle `for (int i = 0; i < v.size(); ++i)` mêlant signé/non signé | warnings, bugs aux tailles > 2³¹ | `std::size_t`, `for (auto& x : v)`, algorithmes |
| 16 | Copie de gros objet en paramètre "par confort" | perf, et dit faux sur la sémantique | `const&` / par valeur si volé / `&&` documenté |
| 17 | `std::vector<bool>` sans savoir pourquoi | proxy bit, pas un `bool&` | `vector<char>`/bitset si le proxy pose problème |
| 18 | Macros pour du code (pas pour l'include-guard ou du codegen) | pas de type, pas de scope, debug horrible | templates, `constexpr`, inline |
| 19 | `volatile` pour la concurrence | n'est PAS une synchronisation (c'est pour le hardware/MMIO) | `std::atomic` |
| 20 | "Smart" lock : `lock()` manuel avec early-returns | deadlock sur le chemin d'erreur | `lock_guard`/`scoped_lock` |
| 21 | Pointeur brut en paramètre *imposé* quand une valeur suffit | restriction gratuite, indirection | par valeur / `const&` |
| 22 | Passer `shared_ptr` par valeur pour lire l'objet | coût atomique + impose l'ownership à l'appelant | `const T&` / `T*` (F.7) |
| 23 | `reinterpret_cast` pour type-punnner | strict aliasing UB | `std::bit_cast` (C++20), `memcpy` bit-safe |
| 24 | Optimisation sans benchmark "parce que c'est évident" | 90% du temps : pire, et illisible | mesurer, garder le bench de régression |
| 25 | Commentaire qui contredit le code | pire que pas de commentaire | supprimer ou corriger ; le code est la source de vérité |
| 26 | Enum C nu (`enum`) | pollution du scope, conversions implicites | `enum class` |
| 27 | `auto` qui masque le type critique (`auto x = get()` renvoyant un proxy) | `auto x = v[0]` copie un proxy, itérateurs invalidés… | `auto&`, `const auto&`, type explicite aux frontières |
| 28 | Lambda `[&]` capturée puis envoyée à un thread/future | dangling le plus classique du langage (F.53) | capture par valeur, `shared_ptr`/`weak_ptr` |
| 29 | Exception traversant une frontière C | terminate garanti | wrapper `noexcept` à la frontière, codes d'erreur C |
| 30 | Réserver à l'usage : `new int[10]` à la place d'un `vector` | reallocation manuelle, pas de taille voyageant avec | conteneurs ; `span` aux frontières |

## Les trois questions qui débusquent 80% des défauts en review

1. **Qui possède cette ressource, et comment le lecteur le sait-il ?** (types)
2. **Que se passe-t-il si cette ligne throw ?** (garanties, RAII)
3. **Cet état peut-il être rendu impossible par le type system ?** (types forts)
