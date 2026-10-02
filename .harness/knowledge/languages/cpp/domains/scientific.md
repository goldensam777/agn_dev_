# C++ Scientifique & Calcul Numérique

> **Idée directrice** : En calcul scientifique, la définition de "qualité" change : un code
> qui compile, passe les tests, et produit un résultat **faux à la 4e décimale** est un
> échec total. La qualité = résultat correct dans la précision visée, reproductible,
> validé par comparaison à une référence, et mesuré — pas supposé rapide.

---

## 1. Arithmétique flottante : les règles de survie

### 1.1 Ce qu'il faut avoir compris une fois pour toutes (Goldberg, "What Every Computer
Scientist Should Know About Floating-Point")

- `float`/`double` = IEEE 754 : représentation binaire inexacte de la plupart des décimaux,
  erreur d'arrondi ~2⁻²⁴ (float) / 2⁻⁵³ (double) par opération.
- L'addition n'est **ni associative ni distributive** en flottant : l'ordre des sommations
  change le résultat. Deux exécutions "équivalentes" (séquentielle vs parallèle) peuvent
  différer légitimement.
- `==` entre flottants calculés est presque toujours un bug. Exception légitime : valeurs
  exactement représentables et opérations exactes (comparaison à 0.0 d'un compteur).
- NaN est *sticky* : une seule opération NaN contamine tout le calcul. Tester en entrée,
  assert sur les invariants physiques (énergie finie, densité ≥ 0).

### 1.2 Cancellation catastrophique — l'erreur qui tue en silence

Soustraire deux grandeurs proches détruit les chiffres significatifs :

```cpp
// Produit en croix : ad − bc avec a·b et c·d grands, résultat petit
// ❌ bruit complet dès que le résultat vrai << grandeurs intermédiaires
double cross(double a, double b, double c, double d) { return a*b - c*d; }

// ✅ la formule FMA de Kahan (pbrt) : erreur bornée à ~2 ulp
inline double diffOfProducts(double a, double b, double c, double d) {
    double cd = c * d;
    double err = std::fma(-c, d, cd);   // erreur exacte de l'arrondi de c*d
    return std::fma(a, b, -cd) + err;
}
```

Cas canoniques et leur parade :
| Formule naïve | Problème | Parade |
|---|---|---|
| `sqrt(b²−4ac)` (équation du 2nd degré) | cancellation quand `b² ≈ 4ac` | racine "safe" par addition de même signe, 2e racine par Viète `x₂ = c/(a·x₁)` |
| Somme de grande série (magnitudes mixtes) | swamping des petits termes | **sommation de Kahan** (compensation), ou Neumaier (améliorée, gère l'addition de sommes partielles) |
| Norme `sqrt(Σx²)` | overflow/underflow intermédiaire | `std::hypot` / scaling |
| Différence de dérivée `(f(x+h)−f(x))/h` | cancellation dès que `h ~ √ε` | complex-step si analytique, extrapolation de Richardson sinon |
| `exp(x) − 1` pour x proche de 0 | cancellation | `std::expm1` (idem `log1p` pour `log(1+x)`) |

**Kahan — à connaître sur le bout des doigts** :
```cpp
double kahanSum(std::span<const double> xs) {
    double sum = 0.0, c = 0.0;                 // c = compensation des bits perdus
    for (double x : xs) {
        double y = x - c;
        double t = sum + y;                    // bas-bits de y perdus ici
        c = (t - sum) - y;                     // … et récupérés ici
        sum = t;
    }
    return sum;
}
```
Coût : 4 additions par élément au lieu d'1. À utiliser quand l'ordre de grandeur des
termes varie fortement ET que la précision compte. Sinon, sommation par paires (pairwise)
ou laisser le compilateur.

### 1.3 Politique de précision du projet (à fixer dans CONVENTIONS.md)

- `double` par défaut ; `float` uniquement là où le volume mémoire/la bande passante
  dominent et la tolérance le permet (documenté par module).
- Tolérances de test explicites et justifiées : `EXPECT_NEAR(got, ref, 1e-10)` avec un
  commentaire *pourquoi* 1e-10. Jamais de tolérance "au pif" copiée entre tests.
- Règle d'or : **toujours comparer à une référence** (solution analytique, autre
  implémentation, valeur publiée dans la littérature) — un test qui compare le code à
  lui-même ne prouve rien.

## 2. Génération aléatoire & reproductibilité

- `<random>` exclusivement : `std::mt19937_64`, distributions (`uniform_real_distribution`,
  `normal_distribution`) — jamais `rand()`.
- **Seed explicite et loguée** : un run scientifique doit être rejouable bit-à-bit. Moteur
  seedé par paramètre de config, seed écrit dans les métadonnées de sortie.
- Attention : distributions identiques + moteurs identiques → résultats identiques ; le
  parallélisme (§4) casse la reproductibilité séquentielle à moins de stratégie de seeding
  par sous-séquence.
- Monte-Carlo : le générateur EST une dépendance scientifique (qualité : mt19937_64 ou
  PCG ; documenter).

## 3. Vectorisation (SIMD) : du gratuit au manuel

### 3.1 Échelle d'intervention (dans l'ordre)

1. **Structure de code auto-vectorisable** (gratuit, portable) :
   - contiguïté (`vector`, `span`, SoA — cf. memory.md) ;
   - indexation linéaire simple ; pas d'aliasing (spans `const`, `__restrict` si prouvé) ;
   - pas de branches dépendantes des données ; pas d'appels non-inline dans la boucle ;
   - pas d'effets de bord globaux.
2. **Flags** : `-O3 -march=native` (GCC/Clang), `/O2 /arch:AVX2` (MSVC). Vérifier le
   résultat avec les rapports de vectorisation (`-fopt-info-vec`, `-Rpass=loop-vectorize`).
3. **Pragmas** : `#pragma omp simd` quand le compilateur hésite.
4. **Bibliothèques** : Eigen (algèbre linéaire), xsimd / `std::experimental::simd`
   (portable), Highway — avant tout intrinsics manuel.
5. **Intrinsics** (`<immintrin.h>` AVX2/AVX-512, NEON) : dernier recours, kernels
   validés, avec chemin scalaire de repli et dispatch par capacités CPU.

### 3.2 Pièges SIMD spécifiques

- **Layout** : le SIMD veut du SoA (`XXXX YYYY ZZZZ`), pas de l'AoS (`XYZR XYZR…`).
  La conversion de layout peut être `constexpr` à la compilation (cf. ray tracer de C. Parker).
- **Branches** : un `if` par lane n'existe pas → masques + blend (`_mm_blendv_ps`,
  conditional move). Comparaison SIMD = masque de bits ; un `if (mask == 0) continue;`
  reste possible (early-out collectif).
- **Réductions** (somme, min…) : coûteuses (shuffles horizontaux) ; les reporter hors de
  la boucle ou les éviter.
- Mesurer : l'auto-vectorisation moderne bat souvent l'intrinsics naïf.

## 4. Parallélisme

| Outil | Quand | Points de qualité |
|---|---|---|
| `std::execution::par_unseq` | One-liner sur algorithms | simple, efficace sur gros volumes |
| OpenMP (`#pragma omp parallel for`) | Boucles numériques chaudes | reduction clause, scheduling statique pour reproductibilité |
| `std::thread` + pools | Tâches hétérogènes | jamais de thread par unité de travail ; pool borné |
| `std::jthread` + `stop_token` | Tâches annulables | cancellation propre = contrat d'API |
| MPI | Multi-nœud (HPC) | hors scope du `.harness` standard ; si utilisé, le déclarer |

**Règles** :
- La granularité : paralléliser les grosses boucles, pas les petites (le sync overhead
  mange le gain en dessous de ~10⁴–10⁵ éléments — à mesurer).
- Réductions parallèles : le résultat dépend de l'ordre → soit l'accepte (tolérance
  documentée), soit reduction déterministe (seeding par bloc, arborescence fixe).
- Toujours benchmarker avec le même dataset que la prod, et garder le benchmark dans
  `bench/` avec un seuil de régression (cf. checklist qualité).

## 5. Tests & validation du code numérique

1. **Tests unitaires** sur des cas à solution connue (exacte ou analytique).
2. **Tests de convergence** : raffiner le pas / le maillage / N et vérifier que l'erreur
   décroît au taux théorique (pente attendue en log-log). C'est LE test qui distingue un
   schéma correct d'un schéma faux qui "a l'air de marcher".
3. **Property-based testing** : invariants physiques (conservation, symétries,
   positivité) pour des milliers d'entrées aléatoires.
4. **Comparaison cross-implémentation** : version de référence lente-simple vs version
   optimisée — la qualité d'une optimisation numérique se prouve par équivalence de
   résultat, pas par "les tests passent".
5. **Bench avec seuils** : Google Benchmark, médiane + p99, rapport en CI, alerte sur
   régression > X%.

## 6. Checklist scientifique

- [ ] Toute soustraction de grandeurs proches identifiée et traitée (FMA, Viète, reformulation)
- [ ] Sommations longues : Kahan/Neumaier ou tolérance justifiée
- [ ] Pas de `==` flottant hors cas exacts ; tolérances explicites et motivées
- [ ] Seed RNG explicite, loguée, reproductible ; `<random>` partout
- [ ] Boucles hot : contiguës, sans aliasing, sans branches données-dépendantes
- [ ] SIMD : structure → flags → pragmas → lib → intrinsics, dans cet ordre, mesuré
- [ ] Parallélisme : granularité juste, réductions et reproductibilité documentées
- [ ] Au moins un test de convergence (ordre théorique vérifié)
- [ ] Référence indépendante pour valider toute optimisation numérique
