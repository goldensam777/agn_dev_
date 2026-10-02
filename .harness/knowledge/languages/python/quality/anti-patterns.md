# Qualité Python : 25 Anti-Patterns Interdits

> Ce document liste les 25 anti-patterns formellement interdits dans le code Python produit par l'agent ou examiné lors de la revue.

---

### 1. Arguments par Défaut Mutables
- ❌ **Interdit :** `def append_item(x: int, items: list[int] = []) -> list[int]: ...` (partagé entre tous les appels).
- ✅ **Exigé :** `def append_item(x: int, items: list[int] | None = None) -> list[int]: items = items if items is not None else []`.

### 2. Capture Aveugle d'Exceptions (Bare Except)
- ❌ **Interdit :** `except:` ou `except Exception: pass` (avale les signaux et masque les bugs).
- ✅ **Exigé :** Capturer les types précis d'exceptions ou toujours consigner / relancer (`logger.exception` ou `raise`).

### 3. Comparaison d'Identité au lieu d'Égalité de Valeur
- ❌ **Interdit :** `if count is 1000:` ou `if name is "forge":` (dépend du caching interne de CPython).
- ✅ **Exigé :** `if count == 1000:` (utiliser `is` uniquement pour les singletons comme `None` et `bool`).

### 4. Modification d'une Collection pendant son Itération
- ❌ **Interdit :** `for item in items: if condition: items.remove(item)` (comportement imprévisible, sauts d'éléments).
- ✅ **Exigé :** `items = [item for item in items if not condition]`.

### 5. Itération par Index `range(len(...))`
- ❌ **Interdit :** `for i in range(len(items)): process(i, items[i])`.
- ✅ **Exigé :** `for i, item in enumerate(items): process(i, item)`.

### 6. Compréhension de Liste Inutile dans les Fonctions d'Agrégation
- ❌ **Interdit :** `sum([x * 2 for x in data])` (alloue une liste complète en mémoire).
- ✅ **Exigé :** `sum(x * 2 for x in data)` (consommation mémoire O(1) via expression génératrice).

### 7. Vérification de Type via `type(x) == Class`
- ❌ **Interdit :** `if type(obj) == Base:` (casse le polymorphisme et ignore les sous-classes).
- ✅ **Exigé :** `if isinstance(obj, Base):`.

### 8. Omission de `slots=True` sur les Classes de Données Multiples
- ❌ **Interdit :** `@dataclass class Node: ...` pour des millions d'instances (surcoût `__dict__`).
- ✅ **Exigé :** `@dataclass(slots=True, frozen=True) class Node: ...`.

### 9. Ouverture de Fichiers sans Context Manager
- ❌ **Interdit :** `f = open("data.bin", "rb"); data = f.read(); f.close()`.
- ✅ **Exigé :** `with open("data.bin", "rb") as f: data = f.read()`.

### 10. Usage de `eval()` ou `exec()` pour du Dispatch Dynamique
- ❌ **Interdit :** `result = eval(f"compute_{method}()")` (faille d'injection et inefficacité du runtime).
- ✅ **Exigé :** Utiliser un dictionnaire de dispatch typé `DISPATCH_MAP: dict[str, Callable[[], float]]`.

### 11. Capture de `BaseException`
- ❌ **Interdit :** `except BaseException:` (intercepte et neutralise `KeyboardInterrupt` et `SystemExit`).
- ✅ **Exigé :** `except Exception:` au maximum.

### 12. Mutation de Variables Globales entre Modules
- ❌ **Interdit :** Utilisation du mot-clé `global` ou modification de variables globales d'un autre module.
- ✅ **Exigé :** Encapsulation dans des classes de contexte passées en paramètre.

### 13. Concaténation de Chaînes dans une Boucle
- ❌ **Interdit :** `s = ""; for line in lines: s += line` (complexité algorithmique O(N^2) par réallocation).
- ✅ **Exigé :** `s = "".join(lines)` (allocation unique O(N)).

### 14. Usage de `assert` pour la Validation Métier en Production
- ❌ **Interdit :** `assert user_input > 0, "Doit être positif"` (les assertions sont désactivées avec `python -O`).
- ✅ **Exigé :** `if user_input <= 0: raise ValueError("Doit être positif")`.

### 15. Masquage Silencieux d'Erreurs (`except: pass`)
- ❌ **Interdit :** `try: do_action() except Exception: pass` (bruit zéro mais corruption silencieuse de l'état).
- ✅ **Exigé :** Consignation explicite `logger.warning(...)` avec justification contextuelle.

### 16. Boucles For Scalaires sur des Tableaux NumPy
- ❌ **Interdit :** Parcourir un tableau NumPy élément par élément avec une boucle for en pur Python.
- ✅ **Exigé :** Utiliser les ufuncs vectorisées (`np.add`, `np.dot`) ou compiler avec Numba `@njit`.

### 17. Copies Involontaires de Tableaux Scientifiques
- ❌ **Interdit :** Effectuer des opérations en chaîne allouant de nouveaux buffers sans réutiliser les buffers existants.
- ✅ **Exigé :** Utiliser le paramètre `out=` dans les ufuncs NumPy.

### 18. Sommeil Bloquant dans du Code Asynchrone
- ❌ **Interdit :** `time.sleep(1)` dans une coroutine `async def` (bloque la boucle d'événements entière).
- ✅ **Exigé :** `await asyncio.sleep(1)`.

### 19. Appel de Code Synchrone Bloquant dans la Boucle Asynchrone
- ❌ **Interdit :** Appeler une fonction de calcul lourd ou une requête réseau synchrone directement dans `async def`.
- ✅ **Exigé :** Déléguer au pool de threads via `await asyncio.to_thread(blocking_func, arg)`.

### 20. Manipulation de Chemins sous Forme de Chaînes de Caractères
- ❌ **Interdit :** `path = directory + "/" + filename` (non-portable sous Windows, failles de séparateurs).
- ✅ **Exigé :** `path = Path(directory) / filename` (`pathlib.Path`).

### 21. Calcul Flottant Instable
- ❌ **Interdit :** `math.log(1.0 + x)` pour des `x` très petits (perte catastrophique de chiffres significatifs).
- ✅ **Exigé :** `math.log1p(x)`.

### 22. Imports Circulaires Monolithiques
- ❌ **Interdit :** Deux modules s'important mutuellement au niveau racine.
- ✅ **Exigé :** Extraire les types partagés dans un module de protocoles ou utiliser `from __future__ import annotations` et `if TYPE_CHECKING:`.

### 23. Imports Typage Obsolètes
- ❌ **Interdit :** `from typing import Optional, List, Dict, Union`.
- ✅ **Exigé :** Utiliser les génériques natifs `list[int]`, `dict[str, float]` et l'opérateur d'union `X | None`.

### 24. Fonctions aux Types de Retour Hétérogènes Imprévisibles
- ❌ **Interdit :** Renvoyer `False` en cas d'erreur et un dictionnaire en cas de succès sans typage précis.
- ✅ **Exigé :** Renvoyer une exception ou un résultat typé `dict[str, object] | None`.

### 25. Rétention Mémoire Involontaire dans les Fermetures (Closures)
- ❌ **Interdit :** Conserver une référence vers un objet géant dans une fermeture alors qu'un seul attribut est nécessaire.
- ✅ **Exigé :** Extraire la valeur scalaire requise avant de définir la fermeture pour permettre au ramasse-miettes (GC) de libérer l'objet lourd.
