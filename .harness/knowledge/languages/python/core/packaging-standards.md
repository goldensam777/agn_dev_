# Core Python : Standards de Packaging & Outillage Moderne

> Norme PEP 621 (`pyproject.toml`), gestionnaire ultra-rapide `uv`, environnements virtuels hermétiques et configuration `ruff` / `mypy`.

---

## 1. Fichier de Configuration Unique : `pyproject.toml` (PEP 621)

Bannir définitivement `setup.py` et `requirements.txt` monolithiques au profit de la norme standard déclarative `pyproject.toml` :

```toml
[build-system]
requires = ["hatchling"]
build-backend = "hatchling.build"

[project]
name = "forge-scientific"
version = "0.1.0"
description = "Calcul scientifique haute précision pour l'environnement Forge"
requires-python = ">=3.12"
authors = [{ name = "Forge Laboratory" }]
dependencies = [
    "numpy>=1.26.0",
    "scipy>=1.12.0",
]

[project.optional-dependencies]
dev = [
    "mypy>=1.9.0",
    "ruff>=0.3.0",
    "pytest>=8.0.0",
]

[tool.ruff]
line-length = 100
target-version = "py312"

[tool.ruff.lint]
select = [
    "E",   # pycodestyle errors
    "W",   # pycodestyle warnings
    "F",   # pyflakes
    "I",   # isort
    "B",   # flake8-bugbear
    "UP",  # pyupgrade
    "C4",  # flake8-comprehensions
    "RUF"  # ruff-specific rules
]

[tool.mypy]
python_version = "3.12"
strict = true
warn_unused_configs = true
disallow_untyped_defs = true
```

---

## 2. Gestionnaire de Dépendances `uv`

L'environnement préconise l'utilisation de `uv` pour une vitesse d'installation 10x à 100x supérieure à `pip` :

```bash
# Création d'un venv hermétique
uv venv .venv
source .venv/bin/activate

# Installation déterministe avec fichier de lock
uv pip install -r requirements.lock
```

---

## 3. Linter & Formatter Haute Vitesse : `ruff`

Ruff remplace `black`, `flake8`, `isort` et `pyupgrade` en un seul binaire natif :

```bash
# Vérification des règles de style et détection des bugs
ruff check .

# Formatage automatique déterministe
ruff format .
```

---

## 4. Checklist Actionnable pour l'Agent

- [ ] Le projet est-il configuré via un fichier `pyproject.toml` standardisé ?
- [ ] Aucun fichier obsolète `setup.py` ou `setup.cfg` n'est-il présent ?
- [ ] Le code est-il formaté et analysé par `ruff` sans aucun avertissement ?
- [ ] Toutes les dépendances sont-elles isolées dans un environnement virtuel hermétique (`.venv`) ?
