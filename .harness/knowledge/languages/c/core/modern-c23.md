# Core C : Le Standard C23 Moderne

> Ce document résume les fonctionnalités clés d'ISO C23 qui modernisent le développement en langage C.

---

## 1. Nouveaux Mots-Clés et Constantes

Le standard C23 introduit des concepts natifs attendus depuis des décennies :

| Fonctionnalité | En C99 / C11 | En C23 (Obligatoire) |
|---|---|---|
| **Pointeur nul** | `NULL` ou `0` (danger d'ambiguïté avec les entiers) | **`nullptr`** (de type dédié `nullptr_t`) |
| **Booléens** | `#include <stdbool.h>` avec macro `bool` | **`bool`**, **`true`**, **`false`** (mots-clés natifs du langage) |
| **Constante compile-time** | `#define CONST 42` ou `enum` hack | **`constexpr double PI = 3.1415926535;`** |
| **Inférence de type** | Déclaration explicite fastidieuse | **`auto x = calculate_complex();`** |
| **Assertion statique** | `_Static_assert(cond, "msg")` | **`static_assert(sizeof(void*) == 8);`** (message optionnel) |

---

## 2. Attributs Standardisés (`[[...]]`)

Plus besoin d'attributs propriétaires non portables comme `__attribute__((...))` ou `__declspec(...)` :
- **`[[nodiscard]]` :** Avertissement immédiat si la valeur de retour est ignorée.
- **`[[maybe_unused]]` :** Indique au compilateur qu'une variable ou fonction peut légitimement ne pas être utilisée (supprime les faux positifs de warnings).
- **`[[unsequenced]]` & `[[reproducible]]` :** Indique des fonctions pures au compilateur pour des optimisations extrêmes de factorisation de code.

---

## 3. Checklist Actionnable pour l'Agent

- [ ] Tout pointeur nul utilise-t-il exclusivement `nullptr` au lieu de `NULL` ?
- [ ] Les assertions de dimensionnement mémoire utilisent-elles `static_assert` ?
- [ ] Les constantes mathématiques et système utilisent-elles `constexpr` au lieu de macros `#define` ?
- [ ] Le code compile-t-il avec `-std=c23` sans warning de dépréciation ?
