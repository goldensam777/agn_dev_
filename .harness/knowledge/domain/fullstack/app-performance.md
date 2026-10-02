# Optimisation d'Applications Fullstack (mesurée, par couche)

> À charger pour toute tâche « rendre l'app plus rapide ». Complète les corpus
> langages : ce document dit QUOI mesurer et QUELS ordres de grandeur viser ;
> les corpus disent COMMENT écrire le code.

---

## 0. La boucle obligatoire (avant toute optimisation)

```
1. MESURER   → baseline chiffrée (pas "ça semble lent")
2. HYPOTHÈSE → la cause racine la plus probable, une seule
3. CORRIGER  → le changement minimal qui attaque cette cause
4. MESURER   → même protocole, même machine, données comparables
5. GARDER    → seulement si gain ≥ seuil convenu (typ. > 10 % sur la métrique cible)
```

Une optimisation sans baseline chiffrée dans la description de tâche n'existe pas.

## 1. Mesure : les métriques qui comptent

### Frontend (Web Vitals — mesure RUM, pas seulement lab)
| Métrique | Cible | Ce qu'elle capte |
|---|---|---|
| LCP (Largest Contentful Paint) | < 2,5 s | vitesse de chargement perçue |
| INP (Interaction to Next Paint) | < 200 ms | réactivité à l'interaction (remplace FID) |
| CLS (Cumulative Layout Shift) | < 0,1 | stabilité visuelle |

Outils : `web-vitals` en RUM, Lighthouse en lab, React DevTools Profiler pour
les renders. Le Profiler React CONFIRME un soupçon de re-render excessif — il
ne justifie jamais du `memo` à l'aveugle.

### Backend
| Métrique | Instrument |
|---|---|
| Latence p50 / p95 / **p99** | APM ou middleware de timing — c'est p99 qui fait mal, pas la moyenne |
| Event loop delay Node | `perf_hooks.monitorEventLoopDelay()` — détecteur n°1 de boucle bloquée |
| Temps de réponse DB par requête | logs de requêtes lentes + EXPLAIN |
| Trafic : req/s, erreurs/s | métriques RED (voir platform-production.md) |

**Règle d'or backend :** si l'event loop delay dépasse ~50 ms en charge, la cause
est presque toujours du travail CPU synchrone dans le process Node (JSON énorme,
regex catastrophique, boucle chaude). La correction est de déplacer ce travail :
`worker_threads`, ou notre propre pont natif (`server/src/bridge/` — c'est
exactement pour ça qu'il existe).

## 2. Backend Node : les gains classiques, par ordre d'impact

1. **Ne jamais bloquer la boucle.** Tout traitement CPU > ~10 ms hors requête :
   worker_threads ou moteur natif. Décision architecturale n°1.
2. **Sérialisation.** Le JSON est le coût caché n°1 des API : gros payloads,
   champs jamais lus, objets profonds. Réponses = DTO plats, champs utilisés.
   Paginer TOUT ce qui peut dépasser quelques centaines d'éléments.
3. **HTTP.** Keep-alive (agent persistant), compression gzip/brotli (seuil ~1-2 ko,
   le CPU de compression se paie — mesurer), `Cache-Control` + ETag/304 sur les
   ressources cachables, HTTP/2 en ingress.
4. **Accès aux données.** Le pattern N+1 est un incident en costume de feature.
   Batch, index sur les colonnes des WHERE réels, projections qui ne ramènent
   que les colonnes lues.
5. **Cache.** Cache-aside avec TTL explicite ET stratégie d'invalidation écrite
   (pas « on verra »). Distinction : cache de données (Redis) vs cache de calcul
   (empreinte → résultat, cf. `scientific-in-production.md`). Un cache sans
   politique d'invalidation est une machine à données pourries.
6. **Backpressure.** Traiter par streams quand le volume est non borné
   (`highWaterMark` respecté) ; mieux vaut répondre 429 qu'empiler 10 000
   requêtes en mémoire (cf. platform-production.md).

## 3. Frontend React : les gains classiques, par ordre d'impact

1. **Le bundle.** Levier n°1 sur LCP/INP. Budget JS gzip (~200 ko exécutés au
   démarrage, à ajuster au projet), code splitting par route (`React.lazy`),
   imports ciblés (jamais de barrel qui traîne toute la lib), analyse régulière
   du bundle (source-map-explorer / Rollup visualizer).
2. **Le serveur d'état.** React Query / TanStack Query : cache serveur avec
   déduplication, revalidation, stale-while-revalidate. Ne jamais re-fabriquer
   ça à la main dans des `useEffect`.
3. **Le rendu.** Composants purs (mêmes props → même rendu) ; mémoïsation SUR
   MESURE et après mesure du Profiler ; listes longues → virtualisation
   (au-delà de ~1 000 lignes, obligatoire) ; `key` stables ; mises à jour non
   urgentes dans `startTransition`.
4. **Les assets.** Images : dimensions fixes dans le JSX (anti-CLS), formats
   AVIF/WebP, lazy below-the-fold, l'image LCP en `<link rel="preload">`.
   Fonts : subset, `font-display: swap`, préchargée.
5. **Le polling.** Jamais de polling < 1 s : WebSocket ou revalidation React Query
   avec backoff. (Voir `canonical_scientific_canvas.tsx` pour le rendu à 60 fps
   hors reconciler.)

## 4. La frontière Web ↔ Serveur

- Valider les réponses avec Zod **côté client aussi** (`web/src/api/client.ts`
  le fait déjà : `ResponseSchema.parse(data)`). Le contrat est une clôture à
  double sens.
- `AbortController` pour annuler les requêtes obsolètes (navigation, frappe
  utilisateur) — une réponse qui arrive trop tard coûte du réseau ET du rendu.
- Taille de payload bornée côté contrat (`contracts/schemas.ts` : `max(10_000_000)`
  sur `dimensions` — même discipline sur les tableaux).

## 5. Anti-patterns interdits (spécifiques à la perf)

1. Optimiser sans baseline chiffrée.
2. `useMemo`/`useCallback`/`memo` posés « par précaution » (lisibilité coûteuse,
   gain non prouvé).
3. Compression activée sur les payloads déjà compressés (images, vidéos).
4. `await` séquentiel de requêtes indépendantes (`Promise.all` par défaut).
5. Re-render d'une liste entière pour un changement sur une ligne (état mal
   localisé — corpus react/core/state-management.md).
6. « On mettra un cache » sans stratégie d'invalidation écrite.

## Checklist avant de clore une tâche perf

- [ ] Baseline chiffrée citée dans la description de tâche
- [ ] Même protocole de mesure avant/après
- [ ] Gain documenté (ou abandon documenté — un « ça ne change rien » mesuré a de la valeur)
- [ ] p99 regardé, pas seulement la moyenne
- [ ] `bash scripts/verify.sh` : exit 0
