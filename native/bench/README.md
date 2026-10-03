# Protocole de Mesure & Baseline de Performance (Mercuria)

Ce répertoire contient le banc de mesure natif et sa référence statistique (*baseline*).

---

## 1. Fichier de référence `baseline.json`

Le fichier [`baseline.json`](./baseline.json) consigne les caractéristiques de la machine de référence et la valeur médiane de débit attendue :
- **Métrique :** Millions d'opérations par seconde (`Mops/s`).
- **Seuil de tolérance :** 5.0 % de régression maximale par rapport à la médiane.
- **Machine de référence actuelle :** 11th Gen Intel(R) Core(TM) i7-1165G7 @ 2.80GHz, 8 cœurs, Fedora Linux.

---

## 2. Vérification de Non-Régression

Le script [`scripts/compare_bench.py`](../../scripts/compare_bench.py) exécute 5 itérations du banc, calcule la médiane pour éliminer les anomalies d'ordonnancement CPU, et vérifie que :
$$\text{Médiane mesurée} \ge \text{baseline\_value} \times (1 - 0.05)$$

Pour tester manuellement :
```bash
python3 scripts/compare_bench.py --runs 5
```

---

## 3. Procédure de Mise à Jour de la Baseline

Lors d'un changement de matériel ou d'une optimisation architecturale durable validée :
```bash
python3 scripts/compare_bench.py --update --runs 5
```
Cette commande exécute le banc, recalcule la médiane, inspecte la machine hôte et met à jour `native/bench/baseline.json`.
