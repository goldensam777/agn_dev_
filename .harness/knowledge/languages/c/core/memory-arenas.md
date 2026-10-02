# Core C : Arènes de Mémoire, Cycle de Vie & Fin du Malloc Dispersé

> Ce document établit la gestion moderne de la mémoire en C. La prolifération de `malloc` et `free` manuels individuels est formellement proscrite.

---

## 1. Pourquoi l'Arène remplace le Malloc/Free Traditionnel

Dans les systèmes à haute performance (moteurs de jeu, compilateurs, calcul scientifique) :
- Les `malloc` individuels provoquent une fragmentation catastrophique et un surcoût métadonnées (16 octets par allocation).
- Les erreurs de `double-free`, `use-after-free` et fuites mémoires proviennent à 90% des désallocations manuelles disséminées.
- **Le modèle par Arène (Bump Allocator) :** Une arène réserve une grande page de mémoire (ex: 64 Ko, 1 Mo) et avance simplement un curseur d'octets. Libération en $O(1)$ en fin de cycle.

```c
typedef struct {
    unsigned char* buffer;
    size_t capacity;
    size_t offset;
} Arena;

Arena arena_create(size_t capacity);
void* arena_alloc(Arena* a, size_t size, size_t align);
void arena_reset(Arena* a); // Remet le curseur à 0 sans désallouer le bloc système
void arena_destroy(Arena* a);
```

---

## 2. Règle de Propriété Explicite en C

Pour les rares cas où une allocation sur le tas système (`malloc`/`calloc`) est indispensable :
1. **La fonction créatrice alloue, la fonction destructrice libère :**
   Toute structure créée par `mon_module_create(...)` doit posséder son pendant exact `mon_module_destroy(...)`.
2. **Assignation de `nullptr` immédiate après `free` :**
   ```c
   free(ptr);
   ptr = nullptr; // Élimine les dangling pointers
   ```

---

## 3. Checklist Actionnable pour l'Agent

- [ ] Les structures de données complexes ou temporaires utilisent-elles une arène mémoire ?
- [ ] Tout pointeur libéré est-il immédiatement assigné à `nullptr` ?
- [ ] Aucun appel à `malloc` n'omet-il de vérifier si le retour est `nullptr` ?
- [ ] L'alignement mémoire est-il respecté lors des allocations personnalisées (`alignof(max_align_t)`) ?
