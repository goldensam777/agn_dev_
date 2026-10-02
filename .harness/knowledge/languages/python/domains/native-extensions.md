# Domaine Python : Extensions Natives C++ (nanobind) & Rust (PyO3)

> Ponts d'interopérabilité FFI haute vitesse, liaison zéro-copie avec NumPy via Buffer Protocol, et libération du GIL.

---

## 1. Choix Technologique pour les Extensions Natives

Bannir `ctypes` ou l'API CPython brute (verbeuse et risquée) au profit des deux standards modernes de l'industrie :

| Langage Natif | Bibliothèque Recommandée | Caractéristiques Clés |
|---|---|---|
| **C++20 / C++23** | **`nanobind`** | Successeur moderne de `pybind11` : binaire 10x plus compact, compilation 4x plus rapide, support natif de `dlpack`. |
| **Rust** | **`PyO3` + `maturin`** | Sécurité mémoire garantie par le compilateur Rust, intégration parfaite avec Cargo et typage automatique. |

---

## 2. Exemple de Liaison C++20 avec `nanobind`

Exposition d'un calcul vectoriel C++ à Python avec libération explicite du GIL :

```cpp
// native_module.cpp
#include <nanobind/nanobind.h>
#include <nanobind/ndarray.h>
#include <span>
#include <numeric>

namespace nb = nanobind;

double fast_accumulate(nb::ndarray<const double, nb::c_contig> array) {
    // Libération du GIL pour permettre l'exécution parallèle sur d'autres threads Python
    nb::gil_scoped_release release;

    std::span<const double> data(array.data(), array.size());
    return std::accumulate(data.begin(), data.end(), 0.0);
}

NB_MODULE(forge_native_cpp, m) {
    m.doc() = "Module natif d'accélération numérique pour la Forge";
    m.def("fast_accumulate", &fast_accumulate,
          nb::arg("array"),
          "Somme ultra-rapide en C++20 sur un tableau contigu sans copie");
}
```

---

## 3. Exemple de Liaison Rust avec `PyO3`

```rust
// lib.rs
use pyo3::prelude::*;
use pyo3::exceptions::PyValueError;
use rayon::prelude::*;

#[pyfunction]
fn parallel_sum_rust(py: Python<'_>, data: Vec<f64>) -> PyResult<f64> {
    if data.is_empty() {
        return Err(PyValueError::new_err("Le tableau ne doit pas être vide"));
    }

    // Libération du GIL et parallélisation massive sur tous les cœurs CPU via Rayon
    let result = py.allow_threads(|| {
        data.par_iter().sum()
    });

    Ok(result)
}

#[pymodule]
fn forge_native_rust(m: &Bound<'_, PyModule>) -> PyResult<()> {
    m.add_function(wrap_pyfunction!(parallel_sum_rust, m)?)?;
    Ok(())
}
```

---

## 4. Partage Zéro-Copie & Protocole de Buffer

- Ne jamais sérialiser en JSON ni copier les données numériques volumineuses entre Python et le module natif.
- Utiliser le **Buffer Protocol** Python standard ou **`dlpack`** pour passer un pointeur brut direct vers la mémoire contiguë.
- S'assurer que le propriétaire de la mémoire reste vivant tant que la vue native ou Python est active (gestion de la durée de vie).

---

## 5. Checklist Actionnable pour l'Agent

- [ ] L'extension native utilise-t-elle `nanobind` (C++) ou `PyO3` (Rust) ?
- [ ] Le GIL est-il explicitement relâché (`gil_scoped_release` ou `allow_threads`) pour les opérations longues ?
- [ ] Les données matricielles transitent-elles en zéro-copie via des vues de pointeurs contigus ?
- [ ] Les erreurs natives sont-elles proprement converties en exceptions Python typées (`PyValueError`, `PyRuntimeError`) ?
