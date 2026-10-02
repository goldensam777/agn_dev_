# Spécification des Messages et Codes d'Erreur (contracts/messages.md)

Ce document formalise les conventions de communication inter-services pour le projet.

---

## 1. Format Standard des Réactions d'Erreur

Toute erreur retournée par l'API backend (`server/`) ou le pont natif doit respecter la structure suivante :

```json
{
  "success": false,
  "error": {
    "code": "CODE_ERREUR_MAJUSCULE",
    "message": "Description claire et lisible du problème.",
    "details": {},
    "timestamp": "2026-10-02T17:00:00.000Z"
  }
}
```

---

## 2. Codes d'Erreur Référencés

| Code d'erreur | Signification | Action attendue |
|---|---|---|
| `VALIDATION_FAILED` | Le payload ne satisfait pas le schéma Zod. | Vérifier le typage dans `contracts/schemas.ts`. |
| `NATIVE_ENGINE_UNAVAILABLE` | Le binaire ou module C++/Rust n'est pas compilé ou a crashé. | Relancer `bash scripts/verify.sh` et vérifier la compilation. |
| `MEMORY_LIMIT_EXCEEDED` | L'algorithme a dépassé le plafond mémoire alloué. | Optimiser l'empreinte mémoire ou réduire les dimensions. |
| `BENCHMARK_REGRESSION` | Baisse de performance > 5% par rapport à la référence. | Profiler le cache L1/L2 avec Mercuria. |
| `INTERNAL_SERVER_ERROR` | Erreur inattendue non gérée. | Consulter les logs de `server/`. |
