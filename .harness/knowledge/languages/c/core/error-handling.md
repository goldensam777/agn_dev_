# Core C : Gestion des Erreurs & Le Pattern RAII `goto cleanup`

> Ce document fixe la gestion des erreurs en C. Le code doit garantir la libération de toutes les ressources même en cas d'échec intermédiaire.

---

## 1. Le Pattern Canonique : `goto cleanup` (Le RAII du C)

En C, sans destructeurs automatiques, les fonctions allouant plusieurs ressources (mémoire, descripteurs de fichiers, verrous) doivent utiliser un point de sortie unique :

```c
[[nodiscard]]
int process_scientific_dataset(const char* filepath, ResultData* out_data) {
    int status = -1;
    FILE* f = nullptr;
    double* buffer = nullptr;

    f = fopen(filepath, "rb");
    if (f == nullptr) {
        return -1; // Rien à libérer ici
    }

    buffer = (double*)malloc(sizeof(double) * 10000);
    if (buffer == nullptr) {
        goto cleanup;
    }

    if (fread(buffer, sizeof(double), 10000, f) != 10000) {
        goto cleanup;
    }

    // Traitement nominal...
    out_data->checksum = compute_checksum(buffer, 10000);
    status = 0; // Succès !

cleanup:
    // Nettoyage centralisé dans l'ordre inverse des acquisitions
    if (buffer != nullptr) {
        free(buffer);
        buffer = nullptr;
    }
    if (f != nullptr) {
        fclose(f);
        f = nullptr;
    }
    return status;
}
```

---

## 2. Le Pattern `Result` en C

Pour les fonctions retournant des données ou une erreur structurée sans effet de bord caché :
```c
typedef enum {
    ERR_OK = 0,
    ERR_INVALID_ARGUMENT,
    ERR_OUT_OF_MEMORY,
    ERR_IO_FAILURE,
} ErrorCode;

typedef struct {
    bool is_ok;
    union {
        double value;
        ErrorCode error;
    };
} DoubleResult;

DoubleResult compute_safe_divide(double numerator, double denominator) {
    if (denominator == 0.0) {
        return (DoubleResult){ .is_ok = false, .error = ERR_INVALID_ARGUMENT };
    }
    return (DoubleResult){ .is_ok = true, .value = numerator / denominator };
}
```

---

## 3. Checklist Actionnable pour l'Agent

- [ ] Toute fonction allouant plus d'une ressource utilise-t-elle le pattern `goto cleanup` pour centraliser la libération ?
- [ ] Les fonctions d'action sont-elles annotées avec `[[nodiscard]]` pour empêcher l'ignorer silencieux d'un code d'erreur ?
- [ ] Aucun `return` prématuré n'est-il placé après une allocation sans passer par la phase de nettoyage ?
