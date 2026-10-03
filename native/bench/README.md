# Protocole de Mesure & Baselines Multi-Machines (Mercuria)

Ce répertoire contient le banc de mesure natif et ses références statistiques (*baselines*).

---

## 1. Architecture des Baselines Multi-Machines

Les architectures matérielles étant hétérogènes (stations locales, serveurs de CI, VM cloud), une mesure de débit numérique ne peut être comparée qu'à une machine de caractéristiques équivalentes.

Le système organise les références par empreinte CPU dans [`native/bench/baselines/`](./baselines/) :
- **Fichier canonique par défaut :** [`native/bench/baseline.json`](./baseline.json)
- **Références dédiées par processeur :** `native/bench/baselines/<cpu_slug>.json`
  - Exemple : [`native/bench/baselines/11th_gen_intel_r_core_tm_i7_1165g7_2_80ghz.json`](./baselines/11th_gen_intel_r_core_tm_i7_1165g7_2_80ghz.json)

---

## 2. Comportement du Comparateur (`scripts/compare_bench.py`)

Lors de l'exécution du juge souverain (`scripts/verify.sh` ou appel direct) :
1. **Machine correspondante :** Si une baseline existe pour le CPU actuel dans `baselines/` ou dans `baseline.json`, le script vérifie la non-régression stricte :
   $$\text{Médiane mesurée} \ge \text{baseline\_value} \times (1 - 0.05)$$
2. **Machine différente (ex: CI GitHub Actions) :** Si la machine actuelle diffère de la référence enregistrée, le script affiche un avertissement visible **« NON COMPARABLE »**, affiche les deux processeurs en vis-à-vis, n'échoue pas arbitrairement, et invite à créer une référence dédiée via `--update`.

---

## 3. Procédure de Création / Mise à Jour d'une Référence

Pour enregistrer ou actualiser la référence sur une machine donnée (5 itérations avec calcul de la médiane) :
```bash
python3 scripts/compare_bench.py --update --runs 5
```
Cette commande génère automatiquement `native/bench/baselines/<cpu_slug>.json` avec les spécifications matérielles complètes et actualise `native/bench/baseline.json`.
