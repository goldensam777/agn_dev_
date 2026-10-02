# Le Calcul Scientifique en Production

> Règles pour servir du calcul scientifique via une plateforme fullstack :
> orchestration de jobs, reproductibilité, sécurité numérique aux frontières,
> monitoring. À croiser avec `fullstack/platform-production.md` (files,
> backpressure, observabilité y sont traités en général ; ici, en version
> scientifique).

---

## 1. Déterminisme & reproductibilité : la promesse minimale

Un résultat scientifique doit être **reproductible** : même entrée → même sortie,
partout, dans 6 mois.

- **Seed explicite, toujours.** `np.random.default_rng(seed)`, `std::mt19937(seed)`,
  et le seed transite dans le contrat ou est journalisé avec le job.
  (`canonical_vectorized_numpy.py` le montre.)
- **Environnement figé par le job :** image Docker (digest), versions des libs,
  paramètres, seed, checksum du dataset → enregistrés AVEC le résultat.
  Sans ce paquet de provenance, un résultat n'est pas une donnée, c'est une rumeur.
- **Pas d'horloge, pas d'ordre de dictionnaire, pas de parallélisme non
  déterministe** dans le chemin qui produit le résultat. Si le parallélisme
  change l'ordre des sommations, le non-déterminisme doit être assumé et borné
  (voir §2).

## 2. Sécurité numérique aux frontières (là où la confiance s'arrête)

1. **Valider les entrées au contrat (Zod) — y compris contre NaN/Inf.** Ajouter
   `.refine(Number.isFinite)` sur tout flottant qui alimente le moteur. Un NaN
   en entrée ne doit jamais devenir un NaN en résultat présenté comme valide.
2. **Rejeter NaN/Inf en SORTIE du moteur natif** avant de répondre à l'utilisateur :
   le binaire C++ doit échouer explicitement plutôt que d'émettre un indéterminé.
3. **Comparaisons avec tolérance, jamais `==`.** Tolérance RELATIVE adossée à
   une référence de précision supérieure (cf. `canonical_kahan_summation.cpp`).
4. **Budget de précision écrit par domaine.** Exemple : « sommes agrégées en
   double compensé (Neumaire) ; matrices ≤ 1e-12 de conditionnement, sinon
   avertissement explicite dans la réponse ». Un budget non écrit = précision
   aléatoire.
5. **Tests de convergence :** pour toute méthode itérative, les golden tests
   vérifient la convergence (résidu décroissant, ordre théorique), pas seulement
   la valeur finale.

## 3. Orchestration des jobs de calcul

La règle d'architecture (cf. platform-production.md §3) devient **absolue** en
scientifique : le calcul ne vit JAMAIS dans la requête HTTP.

```
POST /compute ──► validation Zod ──► file de jobs ──► worker pool ──► binaire natif
                        │                                   │            (timeout,
                        ▼                                   ▼             quota mémoire)
                 202 + jobId (immédiat)              résultat + checksum ──► store
```

- **Idempotence par `jobId` :** rejouer un job produit le même enregistrement,
  pas un doublon. (Le schéma `jobId: z.string().uuid()` est déjà la bonne fondation.)
- **Déduplication par empreinte :** même algorithme + mêmes paramètres + même
  checksum d'entrée → servir le résultat caché. C'est le rôle exact du champ
  `resultChecksum` de `ComputeJobResponseSchema` : il adresse le résultat.
- **Timeouts, annulation, quotas :** chaque job a un timeout dur et une limite
  mémoire (le moteur mesure déjà `peakMemoryBytes` — en faire un quota : un job
  qui dépasse est tué et marqué `failed`, jamais laissé fuir).
- **Retries bornés et déterministes :** en scientifique, un retry est sûr SI le
  job est déterministe (seed fixe) — le dire dans la spec du worker.
- **Backpressure à la soumission :** file pleine → 429 + `Retry-After`, pas une
  file infinie en mémoire.
- **Priorités :** files distinctes (interactif vs batch) pour qu'un batch de
  10 000 jobs ne fasse pas attendre une requête utilisateur.

## 4. Données scientifiques

- **Validation aux frontières, en amont du calcul :** schémas Zod sur tout ce
  qui entre (`contracts/` — déjà la règle d'or du dépôt).
- **Unités dans les types :** un `length_m` n'est pas un `length_mm` (cf.
  branded types, `ts/core/branded-types.md`). En scientifique, une erreur
  d'unité est un incident de production, pas un détail.
- **Versionnage et provenance des datasets :** dataset = contenu adressé
  (checksum) + schéma versionné. Jamais de « le fichier sur le disque du serveur ».
- **Traçabilité du résultat :** qui, quand, quelle version, quels paramètres,
  quelle empreinte d'entrée. Un résultat sans provenance ne se débogue pas et
  ne se publie pas.

## 5. Monitoring scientifique (au-delà des métriques techniques)

- **Les métriques métier sont scientifiques :** précision atteinte, itérations
  de convergence, taux d'échec numérique (NaN/Inf détectés), distribution des
  erreurs vs référence.
- **Le drift se surveille :** si la qualité numérique se dégrade dans le temps
  (mêmes entrées, erreur croissante), c'est un incident au même titre qu'une 500 —
  dépendance mise à jour, compilation différente, donnée corrompue en amont.
- **Le moteur natif est un citoyen observable :** santé (`bridge.isHealthy()`),
  latence par algorithme, débit — dans les métriques RED de la plateforme, pas
  dans un coin du serveur.

## Checklist « feature scientifique en prod »

- [ ] Seed + versions + paramètres + checksum d'entrée journalisés avec le résultat
- [ ] NaN/Inf rejetés à l'entrée (contrat) ET à la sortie (moteur)
- [ ] Comparaisons par tolérance adossée à une référence de précision supérieure
- [ ] Budget de précision écrit pour le domaine
- [ ] Job idempotent (jobId) + déduplication par empreinte
- [ ] Timeout dur + quota mémoire par job
- [ ] Backpressure à la soumission (429 + Retry-After)
- [ ] Golden tests de convergence (pas seulement de valeur finale)
- [ ] `bash scripts/verify.sh` : exit 0
