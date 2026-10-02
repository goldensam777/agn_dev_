# Qualité Python : Checklist de Revue de Code & Outillage

> Grille d'évaluation systématique pour l'agent reviewer et outillage d'audit automatisé pour Python.

---

## 1. Règle de Citation Obligatoire

> **Règle absolue :** Tout commentaire émis lors d'une revue de code doit obligatoirement citer le document et la section exacte du corpus de connaissances (ex: `python/core/memory-datamodels.md §2` ou `python/quality/anti-patterns.md §1`). Une remarque non sourcée est considérée comme nulle.

---

## 2. Grille de Sévérité des Constats

| Niveau | Critères | Conséquence |
|---|---|---|
| **Bloquant (P0)** | Fuite de mémoire/descripteurs, blocage de la boucle `asyncio`, argument mutable par défaut, GIL non relâché dans une extension lourde, exception avalée silencieusement. | Rejet immédiat du PR / correction impérative. |
| **Majeur (P1)** | Boucle `for` scalaire sur des tableaux volumineux, absence de `slots=True`, erreur de typage `mypy`, absence de `raise ... from` lors du re-raise. | Doit être corrigé avant validation. |
| **Mineur (P2)** | Import de types obsolètes (`Optional`, `Union`), concaténation de chaînes sous-optimale non critique, nommage non conforme aux conventions PEP 8. | Suggestion d'amélioration. |

---

## 3. Checklist Systématique en 6 Paliers

### Palier A : Typage & Contrats (`python/core/typing-mypy.md`)
- [ ] Toutes les fonctions publiques et privées ont-elles des annotations complètes ?
- [ ] Les types modernes `X | None` et `list[T]` sont-ils utilisés sans `Optional` ni `Union` ?
- [ ] Le code valide-t-il `mypy --strict` avec 0 avertissement ?

### Palier B : Mémoire & Modèles de Données (`python/core/memory-datamodels.md`)
- [ ] Les classes de données ont-elles `slots=True` (ou `__slots__`) ?
- [ ] Les flux de données de grande taille utilisent-ils des générateurs paresseux ?
- [ ] Y a-t-il zéro liste `list[float]` géante là où un `np.ndarray` s'impose ?

### Palier C : Concurrence & GIL (`python/core/gil-concurrency.md`)
- [ ] Aucune opération bloquante n'est-elle appelée directement dans le thread de la boucle `asyncio` ?
- [ ] Les `TaskGroup` sont-ils utilisés pour l'orchestration concurrente ?
- [ ] Le GIL est-il libéré lors des calculs natifs CPU-intensifs ?

### Palier D : Robustesse & Erreurs (`python/core/error-handling.md`)
- [ ] Y a-t-il zéro clause `except:` nue ou `except Exception: pass` silencieuse ?
- [ ] Toutes les exceptions interceptées puis relancées utilisent-elles `from err` ?
- [ ] Toutes les ressources système (fichiers, connexions) sont-elles encapsulées dans des context managers `with` ?

### Palier E : Calcul Scientifique & Vectorisation (`python/domains/scientific-numpy.md`)
- [ ] Y a-t-il zéro boucle for scalaire sur les matrices ?
- [ ] Les tableaux partagés avec le code natif sont-ils `C_CONTIGUOUS` ?
- [ ] Les opérations récurrentes exploitent-elles le paramètre `out=` pour éliminer les allocations temporaires ?

### Palier F : Extensions Natives (`python/domains/native-extensions.md`)
- [ ] Les extensions C++ utilisent-elles `nanobind` et les extensions Rust `PyO3` ?
- [ ] Le partage de mémoire se fait-il en zéro-copie via le Buffer Protocol ?

---

## 4. Recette Outillage Automatisée

Le code Python doit être validé par la séquence suivante :

```bash
# 1. Analyse statique et respect des conventions PEP 8
ruff check .

# 2. Vérification du formatage
ruff format --check .

# 3. Contrôle strict des types
mypy --strict .

# 4. Tests unitaires et d'intégration
pytest -v --maxfail=1
```
