# Domaine C : Stabilité de l'ABI C & Ponts FFI Polyglottes

> Ce document traite de la conception de frontières d'interfaces binaires stables (C ABI) vers Rust, Node.js et Python.

---

## 1. La C ABI : Le Langage Universel de l'Informatique

La C ABI (Application Binary Interface) est le seul contrat de communication stable entre compilateurs et langages hétérogènes :
- Rust (`extern "C"`), C++, Python (`ctypes`/`cffi`), Node.js (N-API) et Go communiquent tous via l'ABI C.

### Règles d'Or pour une Interface FFI Saine :
1. **Pas de types complexes propriétaires dans les signatures :**
   - N'utiliser que des pointeurs opaques, des types scalaires à taille garantie (`int32_t`, `uint64_t`, `double`), et des structures `#[repr(C)]`.
2. **Gestion de mémoire stricte :**
   - La mémoire allouée par le runtime C doit être libérée par le runtime C. Ne jamais laisser un autre langage appeler son propre `free()` sur un pointeur C.
3. **Protection contre le mangling C++ :**
   - Tout header C susceptible d'être inclus dans du C++ doit comporter le garde `extern "C"` :
     ```c
     #ifdef __cplusplus
     extern "C" {
     #endif

     void my_c_function(void);

     #ifdef __cplusplus
     }
     #endif
     ```

---

## 2. Checklist Actionnable pour l'Agent

- [ ] Les fonctions exportées FFI utilisent-elles des types à largeur fixe issus de `<stdint.h>` ?
- [ ] Les fichiers headers possèdent-ils le garde `extern "C"` pour la compatibilité C++ ?
- [ ] Chaque objet alloué via l'ABI possède-t-il sa fonction `_destroy` / `_free` correspondante ?
