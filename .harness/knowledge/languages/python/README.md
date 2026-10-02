# Corpus de Connaissances Python (3.12 / 3.13 / 3.14) — Pour Agents de Développement

> Corpus de référence interne du `.harness/knowledge/languages/python/`. Encode le calcul scientifique haute performance, l'interopérabilité native (PyO3 / nanobind), le typage statique strict et la maîtrise du GIL / Free-threading. **À charger sélectivement** : le `core/` systématiquement ; les `domains/` selon la tâche ; `quality/` pour le reviewer.

---

## 1. Structure du Corpus

```
python-knowledge/
├── README.md                    ← Ce fichier (index + règles de chargement)
├── core/                        ← SOCLE SYSTÉMATIQUE (lu pour toute tâche Python)
│   ├── typing-mypy.md           Typage strict PEP 695/585, Generic, Union |, mypy strict
│   ├── memory-datamodels.md     Surcoût PyObject, __slots__, dataclass(slots=True), générateurs
│   ├── gil-concurrency.md       Gestion du GIL, Python 3.13 no-gil, multiprocessing shared_memory
│   ├── error-handling.md        Hiérarchie d'exceptions, context managers with, raise ... from
│   └── packaging-standards.md   Standard pyproject.toml (PEP 621), uv/venv, dépendances hermétiques
├── domains/                     ← CHARGÉ À LA DEMANDE selon la tâche
│   ├── scientific-numpy.md      Vectorisation NumPy, agencement mémoire C vs Fortran, vues zéro-copie
│   ├── native-extensions.md     Extensions C++ (nanobind) & Rust (PyO3), libération du GIL
│   └── numerical-gpu.md         Accélération JIT (JAX, Numba @njit, PyTorch), calcul tensoriel
└── quality/                     ← CHARGÉ PAR LE REVIEWER
    ├── anti-patterns.md         25 anti-patterns interdits (arguments mutables par défaut, bare except...)
    └── review-checklist.md      Checklist de review + linter ruff & mypy --strict
```

---

## 2. Règles de Chargement (Matrice de Routage)

| Situation / Rôle de l'Agent | Fichiers à charger |
|---|---|
| **Toute tâche Python** | `core/*` (5 fichiers fondamentaux) |
| **Calcul matriciel / Analyse NumPy / SciPy** | `core/*` + `domains/scientific-numpy.md` |
| **Création d'extensions C++ / Rust (FFI)** | `core/*` + `domains/native-extensions.md` |
| **GPU / Machine Learning / JIT JAX** | `core/*` + `domains/numerical-gpu.md` |
| **Phase de Review / Contrôle Qualité** | `quality/*` (+ le domaine concerné) |

---

## 3. Principes Transverses de Python Scientifique d'Élite

1. **Interdiction des boucles `for` en pur Python sur les données massives :** Toujours vectoriser les opérations avec NumPy, JAX ou déléguer au cœur natif C++/Rust.
2. **Typage Statique Strict :** Tout module Python doit être 100% annoté avec les types modernes (`int | None`, `list[T]`) et validé sous `mypy --strict`.
3. **Élimination du Surcoût Mémoire :** Les classes de données doivent systématiquement utiliser `@dataclass(slots=True)` pour éliminer le dictionnaire d'instance `__dict__`.
4. **Libération du GIL lors du Calcul Lourd :** Toute extension native C++ ou Rust effectuant un calcul intensif doit libérer le GIL (`py.allow_threads`) pour permettre l'exécution parallèle sur tous les cœurs CPU.
