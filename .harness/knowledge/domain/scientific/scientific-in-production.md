# Domaine Scientifique : Calcul Numérique en Production

> Rigueur numérique, reproductibilité déterministe, sécurité aux frontières d'interfaces, orchestration des calculs lourds et monitoring du drift scientifique.

---

## 1. Reproductibilité & Traçabilité de Provenance

Un calcul scientifique en production doit pouvoir être rejoué à l'identique plusieurs mois plus tard, sur une autre machine :

### A. Gestion Déterministe des Graines Aléatoires (Seeds) :
- Ne jamais appeler `np.random.seed()` globalement (effet de bord inter-modules imprévisible).
- Toujours instancier un générateur explicite isolé :
  ```python
  rng = np.random.default_rng(seed=42)
  samples = rng.standard_normal(size=10_000)
  ```

### B. Empreinte de Provenance (Provenance Tracking) :
Chaque résultat de calcul doit être scellé avec un manifeste incluant :
- Le hash SHA-256 des données d'entrée.
- La version exacte de l'algorithme et du commit Git.
- Les drapeaux de compilation du moteur natif (`-O3`, `-march=native`, options d'arrondi FMA).
- L'architecture matérielle (ex: x86_64 AVX-512 vs ARM64 NEON).
- Le `resultChecksum` de validation croisée (déjà encodé dans nos contrats `contracts/schemas.ts`).

---

## 2. Sécurité Numérique aux Frontières (Zéro NaN / Zéro Inf)

Les valeurs non numériques (`NaN`) et infinies (`±Inf`) ont la propriété délétère de contaminer silencieusement l'ensemble des matrices lors des multiplications en cascade.

### Règle d'or aux frontières d'API :
1. **Validation en Entrée :** Les validateurs Zod dans `contracts/schemas.ts` et les assertions C++/Rust doivent rejeter immédiatement toute valeur non finie (`std::isfinite(val)` ou `z.number().finite()`).
2. **Validation en Sortie :** Aucun moteur natif ne doit renvoyer de payload contenant un `NaN` non détecté. En cas d'instabilité, lever une exception typée `ConvergenceError` plutôt que de polluer les pipelines d'affichage.

### Assertions Flottantes Tolérantes aux Arrondis :
Ne jamais tester l'égalité stricte `a == b` sur des nombres à virgule flottante :
$$\text{Tolérance} : |a - b| \le \text{atol} + \text{rtol} \times |b|$$
Utiliser systématiquement `np.testing.assert_allclose(actual, desired, rtol=1e-7, atol=1e-9)` en Python, ou `std::abs(a - b) <= (atol + rtol * std::abs(b))` en C++.

---

## 3. Orchestration des Jobs de Calcul Lourd

1. **Découplage Temporel (Asynchronisme) :**
   - Une requête HTTP ne doit jamais bloquer pendant l'exécution d'un calcul numérique de plus de 200 ms.
   - Modèle asynchrone exigé :
     $$\text{Client} \xrightarrow{\text{POST /compute}} \text{Job Enregistré (HTTP 202 + JobId)} \to \text{File de Tâches} \to \text{Worker Pool C++/Rust}$$
     Le client consulte la progression via polling SSE ou WebSockets.
2. **Idempotence & Déduplication :**
   - L'identifiant de job (`jobId`) est calculé comme le hash cryptographique des paramètres d'entrée.
   - Si deux requêtes identiques arrivent simultanément, le worker mutualise le calcul et retourne le même résultat sans recalcul inutile.
3. **Quotas Mémoire & Isolation Processus :**
   - L'exécution du moteur natif est confinée avec des limites d'adresses virtuelles (`RLIMIT_AS`) ou sous cgroups pour éviter qu'une instabilité mémoire n'abatte l'orchestrateur Node.js.

---

## 4. Surveillance du « Drift » Scientifique

En environnement de production, les distributions de données réelles dérivent (*data drift*) et peuvent pousser les algorithmes vers des zones de conditionnement instable :

- **Conditionnement Matriciel :** Surveiller périodiquement le nombre de condition $\kappa(A) = \|A\| \cdot \|A^{-1}\|$. Si $\kappa(A) > 10^{12}$ en simple ou double précision, déclencher une alerte d'instabilité numérique imminente.
- **Norme des Résidus :** Consigner la norme du résidu $\|Ax - b\|$ à chaque pas de résolution linéaire pour détecter une perte de convergence.
- **Monitoring Temporel :** Tracer l'évolution des distributions statistiques (moyenne, écart-type, percentiles 1% et 99%) pour détecter les dérives de capteurs ou de flux d'entrée.
