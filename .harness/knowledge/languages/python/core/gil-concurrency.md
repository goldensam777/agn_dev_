# Core Python : Concurrence, Parallélisme & Gestion du GIL

> Modèle de concurrence Python : CPython GIL, Python 3.13+ Free-Threading, `asyncio` pour les I/O et parallélisme multi-processus avec mémoire partagée.

---

## 1. Comprendre le Global Interpreter Lock (GIL)

Le GIL de CPython empêche l'exécution simultanée de bytecode Python par plusieurs threads natifs au sein d'un même processus.

### Règle d'or de sélection du modèle :
| Type de charge | Modèle recommandé | Outil |
|---|---|---|
| **I/O-Bound (Réseau, HTTP, WebSockets)** | Concurrence asynchrone mono-thread | `asyncio` / `aiohttp` / `fastapi` |
| **CPU-Bound pur Python** | Parallélisme multi-processus | `concurrent.futures.ProcessPoolExecutor` |
| **CPU-Bound haute performance** | Cœur natif libérant le GIL ou Free-Threading | C++ (`nanobind`), Rust (`PyO3`) |
| **Python 3.13+ Free-Threading (PEP 703)** | Multi-threading réel sans GIL | Build CPython `nogil` (`sys._is_gil_enabled() == False`) |

---

## 2. Concurrence Asynchrone : `asyncio`

Pour les services d'orchestration ou les appels réseau à haute concurrence :

```python
import asyncio
from typing import Final

TIMEOUT_SECONDS: Final[float] = 5.0

async def fetch_telemetry(endpoint: str) -> dict[str, object]:
    # Simulation d'un appel réseau asynchrone non-bloquant
    await asyncio.sleep(0.01)
    return {"endpoint": endpoint, "status": "ok"}

async def gather_metrics(endpoints: list[str]) -> list[dict[str, object]]:
    async with asyncio.TaskGroup() as tg:
        tasks = [tg.create_task(fetch_telemetry(ep)) for ep in endpoints]
    return [task.result() for task in tasks]
```

> **Important :** Utiliser `asyncio.TaskGroup` (Python 3.11+) plutôt que `asyncio.gather` pour garantir l'annulation propre et la propagation immédiate des erreurs.

---

## 3. Parallélisme Multi-Processus & Mémoire Partagée

Pour exécuter du calcul intensif en pur Python sans subir le surcoût de sérialisation `pickle` entre processus, utiliser `multiprocessing.shared_memory` :

```python
from multiprocessing import shared_memory
import numpy as np

def create_shared_matrix(rows: int, cols: int) -> tuple[shared_memory.SharedMemory, np.ndarray]:
    nbytes = rows * cols * 8  # float64
    shm = shared_memory.SharedMemory(create=True, size=nbytes)
    # Création d'une vue NumPy directe sur le bloc de mémoire partagée
    arr = np.ndarray((rows, cols), dtype=np.float64, buffer=shm.buf)
    return shm, arr

def cleanup_shared_memory(shm: shared_memory.SharedMemory) -> None:
    shm.close()
    shm.unlink()
```

---

## 4. Libération du GIL dans le Code Natif

Tout code natif effectuant une boucle de calcul lourd doit impérativement libérer le GIL pour libérer l'interpréteur Python :

- **C++ (nanobind) :**
  ```cpp
  nb::call_guard<nb::gil_scoped_release>()
  ```
- **Rust (PyO3) :**
  ```rust
  py.allow_threads(|| {
      // Calcul multithreadé intensif (ex: Rayon)
  })
  ```

---

## 5. Checklist Actionnable pour l'Agent

- [ ] Les calculs CPU-intensifs évitent-ils de faire tourner du code pur Python dans plusieurs threads standards ?
- [ ] Le code réseau et I/O utilise-t-il `asyncio.TaskGroup` ?
- [ ] Les échanges de gros tableaux entre processus utilisent-ils `multiprocessing.shared_memory` pour éviter la sérialisation ?
- [ ] Les extensions natives C++/Rust libèrent-elles explicitement le GIL lors des opérations longues ?
