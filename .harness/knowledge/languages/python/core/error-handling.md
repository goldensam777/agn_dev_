# Core Python : Gestion des Erreurs & Robustesse

> Hiérarchie d'exceptions typées, chaînage d'exceptions (`raise ... from`), context managers déterministes et groupes d'exceptions (PEP 654).

---

## 1. Hiérarchie d'Exceptions Structurée

Ne jamais lever de simples `RuntimeError` ou `Exception` génériques. Définir une arborescence dédiée au domaine :

```python
class ForgeError(Exception):
    """Classe de base pour toutes les erreurs de l'environnement Forge."""

class ComputeError(ForgeError):
    """Erreur survenue lors de l'exécution d'un calcul numérique."""

class ConvergenceError(ComputeError):
    """Échec de convergence de l'algorithme itératif."""
    def __init__(self, iterations: int, residual: float) -> None:
        super().__init__(f"Non-convergence après {iterations} itérations (résidu={residual:.2e})")
        self.iterations = iterations
        self.residual = residual

class ValidationError(ForgeError):
    """Erreur de validation de schéma ou de données en entrée."""
```

---

## 2. Chaînage Explicite des Exceptions (`raise ... from`)

Préserver impérativement la cause racine lors de la traduction d'exceptions pour garantir un diagnostic précis :

```python
import json

def parse_configuration(raw_payload: str) -> dict[str, object]:
    try:
        data = json.loads(raw_payload)
        if not isinstance(data, dict):
            raise TypeError("Le payload JSON doit être un objet.")
        return data
    except (json.JSONDecodeError, TypeError) as err:
        # Préservation explicite de la cause originelle via 'from err'
        raise ValidationError("Configuration invalide ou corrompue.") from err
```

> **Anti-pattern absolu :** Le *bare except* (`except:`) ou `except Exception:` silencieux sans logging ni re-raise.

---

## 3. Context Managers Déterministes (RAII en Python)

Garantir la libération des ressources système (fichiers, sockets, locks) même en cas d'exception non interceptée :

```python
from collections.abc import Generator
from contextlib import contextmanager
import time

@contextmanager
def execution_timer(operation_name: str) -> Generator[None, None, None]:
    start = time.perf_counter_ns()
    try:
        yield
    finally:
        duration_ms = (time.perf_counter_ns() - start) / 1_000_000.0
        print(f"[{operation_name}] Exécuté en {duration_ms:.3f} ms")
```

---

## 4. Groupes d'Exceptions et Syntaxe `except*` (PEP 654)

Utilisé pour capturer les erreurs multiples levées par des tâches concurrentes (`TaskGroup`) :

```python
async def run_batch_operations() -> None:
    try:
        async with asyncio.TaskGroup() as tg:
            tg.create_task(operation_one())
            tg.create_task(operation_two())
    except* ValidationError as eg:
        # Gère toutes les erreurs de validation du groupe
        for err in eg.exceptions:
            logger.error(f"Validation échouée : {err}")
    except* ComputeError as eg:
        # Gère toutes les erreurs de calcul du groupe
        for err in eg.exceptions:
            logger.critical(f"Calcul critique échoué : {err}")
```

---

## 5. Checklist Actionnable pour l'Agent

- [ ] Les exceptions définies héritent-elles d'une classe de base explicite du module ?
- [ ] Les traductions d'exceptions utilisent-elles la clause `from err` ?
- [ ] Y a-t-il zéro bloc `except:` nu ou silencieux ?
- [ ] Toutes les ressources acquises sont-elles libérées via des context managers (`with`) ?
