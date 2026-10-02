# Domaine Transverse : Scientifique en Production

> À charger pour toute tâche qui met du calcul scientifique entre les mains
> d'utilisateurs réels : pipelines de jobs, résultats numériques servis par API,
> reproductibilité, monitoring scientifique. Complète les corpus langages
> (cpp/domains/scientific.md, python/domains/scientific-numpy.md…) : ceux-ci
> disent comment écrire le calcul ; ce document dit comment le FAIRE TOURNER.

## Structure

```
scientific/
├── README.md                    ← ce fichier
└── scientific-in-production.md  ← jobs, reproductibilité, sécurité numérique en prod
```

## Règles de chargement

| Situation | Charger |
|---|---|
| Implémenter un algorithme / optimiser du calcul | corpus langage (`domains/scientific*`) |
| Servir du calcul via l'API, orchestrer des jobs, garantir la reproductibilité | `scientific-in-production.md` |
| Review d'un résultat numérique exposé aux utilisateurs | `scientific-in-production.md` + `quality/` du langage |

## Principe fondateur

En scientifique, la qualité a une définition plus dure qu'ailleurs : **un résultat
presque correct est une réponse fausse avec de bonnes manières.** Un crash se voit ;
un résultat numériquement faux, non. Toute la discipline vient de là.
