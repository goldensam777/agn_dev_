# Core Python : Typage Statique Moderne & Validation Mypy

> Ce document établit les standards de typage strict en Python (PEP 585, PEP 604, PEP 695).

---

## 1. Syntaxe de Typage Moderne (Python 3.10+)

Bannir les imports obsolètes du module `typing` (`List`, `Dict`, `Union`, `Optional`) au profit des types natifs :

| Ancien style (Obsolète) | Nouveau standard moderne |
|---|---|
| `from typing import List, Optional, Union` | *Aucun import nécessaire* |
| `Optional[str]` | `str | None` |
| `Union[int, float]` | `int | float` |
| `List[int]` | `list[int]` |
| `Dict[str, Any]` | `dict[str, object]` (ou typage précis) |

---

## 2. Définition de Type Générique (PEP 695 - Python 3.12+)

```python
# Déclaration élégante de types génériques sans TypeVar verbeux
type Matrix[T: (int, float)] = list[list[T]]

def transpose[T](matrix: list[list[T]]) -> list[list[T]]:
    return [list(row) for row in zip(*matrix, strict=True)]
```

---

## 3. Typage Structurel avec `Protocol`

Permet d'exiger des comportements (*duck typing* statique) sans forcer l'héritage :
```python
from typing import Protocol

class ComputeEngine(Protocol):
    def compute(self, iterations: int) -> float: ...

def run_benchmark(engine: ComputeEngine) -> None:
    result = engine.compute(100_000)
    print(f"Résultat : {result}")
```

---

## 4. Checklist Actionnable pour l'Agent

- [ ] Toutes les fonctions possèdent-elles des annotations de types complètes (arguments et retour) ?
- [ ] La syntaxe moderne `A | B` et `list[T]` est-elle utilisée sans `Union` ni `Optional` ?
- [ ] Le code passe-t-il sans avertissement sous `mypy --strict` ?
