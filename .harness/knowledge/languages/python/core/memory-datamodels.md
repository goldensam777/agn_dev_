# Core Python : Modèles de Données & Optimisation Mémoire

> Maîtrise de l'agencement mémoire CPython, élimination du surcoût `PyObject`, utilisation de `__slots__` et structures efficaces en cache.

---

## 1. Anatomie d'un `PyObject` et Surcoût Mémoire

En CPython, le moindre entier ou flottant est une structure C complète (`PyObject` ou `PyVarObject`) contenant :
- `ob_refcnt` (8 octets sous architecture 64-bit) : compteur de références.
- `ob_type` (8 octets) : pointeur vers le type (`PyTypeObject`).
- La valeur réelle (8 octets pour un `float`, variable pour un `int` arbitraire).

Un simple entier en Python pèse typiquement **28 octets**, et un `float` pèse **24 octets**, là où un `double` en C++ ou Rust ne prend que **8 octets**.

### Conséquence :
- Une liste `list[float]` de 1 million d'éléments ne contient pas des nombres contigus, mais un tableau de pointeurs vers 1 million d'objets distincts dispersés dans le tas (*cache thrashing* massif).
- Pour des données scalaires volumineuses, utiliser impérativement **NumPy arrays**, **`array.array`** ou **`bytes`**.

---

## 2. Élimination du Dictionnaire d'Instance : `__slots__` & Dataclasses

Par défaut, chaque instance de classe Python possède un dictionnaire `__dict__` alloué dynamiquement (~104 octets par instance) pour stocker ses attributs, plus une table de hachage.

### Règle absolue :
Utiliser systématiquement `slots=True` avec `@dataclass` (Python 3.10+) ou `__slots__` explicite pour les structures instanciées fréquemment.

```python
from dataclasses import dataclass

# INVALIDE : Alloue un __dict__ par instance, surcoût massif
@dataclass
class ParticleNaive:
    x: float
    y: float
    z: float

# VALIDE : Aucune allocation de __dict__, attributs mappés en offsets C fixes
@dataclass(slots=True, frozen=True)
class ParticleOptimized:
    x: float
    y: float
    z: float
```

**Gain mesuré :** ~60% d'économie mémoire et ~20% d'accélération sur les accès aux attributs.

---

## 3. Évaluation Paresseuse : Générateurs vs Listes

Pour traiter des flux de données ou de grands volumes :
- **Bannir :** La création de listes intermédiaires complètes en mémoire (`[transform(x) for x in huge_dataset]`).
- **Exiger :** Les expressions génératrices `(transform(x) for x in huge_dataset)` et les générateurs avec `yield`.

```python
from collections.abc import Iterator

def read_large_dataset(filepath: str) -> Iterator[str]:
    with open(filepath, "r", encoding="utf-8") as f:
        for line in f:
            stripped = line.strip()
            if stripped and not stripped.startswith("#"):
                yield stripped
```

---

## 4. Mesure et Audit Mémoire

Ne jamais deviner : mesurer la consommation réelle via :
- `sys.getsizeof(obj)` (taille superficielle de l'objet direct).
- Le module standard `tracemalloc` pour détecter les allocations anormales et les fuites de références cycliques.

```python
import tracemalloc

tracemalloc.start()
# ... exécution de la charge de calcul ...
current, peak = tracemalloc.get_traced_memory()
tracemalloc.stop()
print(f"Pic mémoire : {peak / (1024 * 1024):.2f} Mo")
```

---

## 5. Checklist Actionnable pour l'Agent

- [ ] Les classes et modèles de données possèdent-ils `slots=True` ou `__slots__` ?
- [ ] Les flux de données de grande taille utilisent-ils des générateurs / itérateurs plutôt que des listes en mémoire ?
- [ ] Les tableaux de scalaires massifs évitent-ils `list[float]` au profit de `numpy.ndarray` ou `array.array` ?
- [ ] Les fichiers et descripteurs sont-ils hermétiquement fermés via des context managers `with` ?
