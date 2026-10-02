# Core C : Conception d'Interfaces & Encapsulation Opaque

> Ce document établit les règles d'architecture d'API pour les bibliothèques et modules C.

---

## 1. Structures Opaques (L'Équivalent du PImpl en C)

Pour garantir la stabilité de l'ABI et empêcher les clients d'accéder aux détails internes d'un moteur :
- **Dans le fichier d'en-tête (`engine.h`) :** Déclarer uniquement le type pointeur sans révéler les champs de la structure.
  ```c
  // engine.h
  typedef struct Engine Engine;

  [[nodiscard]] Engine* engine_create(void);
  void engine_destroy(Engine* e);
  int engine_step(Engine* e, double dt);
  ```
- **Dans le fichier d'implémentation (`engine.c`) :** Définir la structure complète.
  ```c
  // engine.c
  struct Engine {
      Arena memory_pool;
      double current_time;
      size_t state_flags;
  };
  ```

---

## 2. Tableaux Dimensionnés Stricts (`[static n]`)

En C moderne, pour garantir au compilateur qu'un buffer passé en paramètre possède au moins $N$ éléments non nuls :
```c
// Indique formellement au compilateur que 'data' a AU MOINS 'n' éléments non nuls
void compute_vector_sum(size_t n, const double data[static n], double* out_result);
```
*Le compilateur peut alors émettre des avertissements si un buffer trop court est passé et optimiser les boucles sans vérification redondante.*

---

## 3. Checklist Actionnable pour l'Agent

- [ ] Les structures internes d'un module sont-elles opaques dans les fichiers headers publics ?
- [ ] Tous les paramètres de lecture seule utilisent-ils `const` de manière rigoureuse ?
- [ ] Les fonctions dont l'ignorer du retour est un bug utilisent-elles `[[nodiscard]]` ?
- [ ] Les pointeurs de tableaux de taille fixe exploitent-ils la notation `[static n]` ?
