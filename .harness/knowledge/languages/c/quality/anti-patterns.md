# Quality C : Catalogue des 25 Anti-Patterns Interdits

> Tout code enfreignant l'une de ces 25 règles sera rejeté lors de la revue et du contrôle de qualité.

---

### Catégorie A : Vulnérabilités Mémoire & Sécurité
1. **Usage de fonctions de chaînes non bornées :** `gets()`, `strcpy()`, `strcat()`, `sprintf()`. *Remède : `snprintf` exclusivement.*
2. **`malloc` sans vérification :** Oublier de tester `if (ptr == nullptr)`.
3. **Double Free :** Appeler `free()` deux fois sur le même pointeur.
4. **Use-After-Free :** Accéder à un pointeur après libération. *Remède : assigner systématiquement `ptr = nullptr`.*
5. **Fuite mémoire sur sortie prématurée :** Faire un `return` d'erreur avant de libérer les ressources allouées. *Remède : `goto cleanup`.*
6. **Arithmétique sur `void*` :** `ptr + 1` sur un pointeur générique (interdit en ISO C). *Remède : caster en `unsigned char*` ou `uint8_t*`.*
7. **Buffer géant sur la pile :** `char buf[1024 * 1024]` sur la stack (risque de stack overflow). *Remède : allocation via arène ou `mmap`.*

### Catégorie B : Comportements Indéterminés (UB)
8. **Violation de Strict Aliasing :** Caster un `int*` en `float*` pour lire ses octets. *Remède : `memcpy`.*
9. **Débordement d'entier signé :** Compter sur un overflow de `int32_t` pour boucler.
10. **Décalage de bits invalide :** Décaler d'un nombre de bits $\ge$ à la taille du type (`x << 32` sur `uint32_t`).
11. **Déréférencement non aligné :** Lire un `uint64_t` à une adresse impaire.
12. **Variable non initialisée :** Déclarer sans valeur initiale (`int x;`).

### Catégorie C : Qualité C23 & Style
13. **Usage de `NULL` au lieu de `nullptr` :** Utiliser la macro obsolète en C23.
14. **Cast du retour de `malloc` :** `(int*)malloc(...)` (inutile en C et masque l'oubli de `<stdlib.h>`).
15. **Macros de calcul non parenthésées :** `#define SQ(x) x * x` (provoque des bugs d'évaluation de priorité).
16. **Macros pour des constantes au lieu de `constexpr` :** Utiliser `#define PI 3.14` au lieu de `constexpr double PI = 3.14;`.
17. **Omission de `[[nodiscard]]` :** Ne pas avertir quand le code d'erreur d'une fonction critique est ignoré.

### Catégorie D : Système & Flottants
18. **Descripteurs sans `O_CLOEXEC` :** Risque de fuite de socket/fichier vers un processus enfant.
19. **Appel non-réentrant dans un signal handler :** Appeler `malloc` ou `printf` dans un gestionnaire de signal.
20. **Égalité stricte sur les flottants :** `a == b` sur `double`.
21. **Omission du zéro terminal `\0` :** Créer une chaîne sans s'assurer que le dernier octet est `\0`.
22. **`sizeof` sur le mauvais type :** `malloc(sizeof(ptr))` au lieu de `malloc(sizeof(*ptr) * count)`.
23. **Compilation avec désactivation de warnings :** Tolérer le moindre avertissement du compilateur.
24. **Dépassement d'indice de tableau :** Écrire sur `arr[n]` pour un tableau de taille $n$.
25. **Absence du mot-clé `restrict` sur les buffers de calcul :** Ralentit le débit de calcul par 2 à 4 fois.
