# Domaine Transverse : Applications & Plateformes Fullstack en Production

> Corpus transverse, au-dessus des corpus langages. À charger UNIQUEMENT quand la
> tâche dépasse le périmètre d'un seul langage : « optimiser l'app », « mettre en
> prod », « scaler la plateforme », « fiabiliser un déploiement ».
> Pour écrire du code dans UN langage → corpus `.harness/knowledge/languages/<lang>/`.

## Structure

```
fullstack/
├── README.md                 ← ce fichier (index + règles de chargement)
├── app-performance.md        ← optimiser une app fullstack (mesuré, par couche)
└── platform-production.md    ← faire tourner une PLATEFORME fullstack en prod
```

## Règles de chargement (routage par le lead/playbook)

| Situation | Charger |
|---|---|
| Tâche « rendre plus rapide » (sans mise en prod) | `app-performance.md` |
| Tâche « déployer / scaler / fiabiliser / monitorer » | `platform-production.md` |
| Les deux (refonte perf avant montée en charge) | les deux, dans cet ordre |
| Review d'une modif touchant perf ou prod | le document pertinent + `quality/` du langage |

## Principe fondateur (non négociable)

**On n'optimise rien qu'on n'a pas mesuré.** CONVENTIONS.md : « performance mesurée,
jamais présumée ». Toute recommandation de ces documents s'applique APRÈS capture
d'une métrique de référence, et toute optimisation livrée DOIT montrer le avant/après.

Le juge local reste `scripts/verify.sh` ; les juges de prod sont les métriques
(voir `platform-production.md` → Observabilité).
