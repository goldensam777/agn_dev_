#!/usr/bin/env python3
"""
scripts/compare_bench.py — Comparateur de performance par rapport à la baseline

Exécute le banc de mesure plusieurs fois, calcule la médiane pour éliminer le bruit,
et vérifie qu'aucune régression de performance supérieure au seuil (ex: 5%) n'est survenue.
"""

from __future__ import annotations
import argparse
import json
import re
import statistics
import subprocess
import sys
from pathlib import Path

def run_benchmark(bin_path: Path, runs: int) -> list[float]:
    values: list[float] = []
    pattern = re.compile(r"Débit\s*:\s*([0-9.]+)\s*Millions", re.IGNORECASE)

    for i in range(1, runs + 1):
        result = subprocess.run([str(bin_path)], capture_output=True, text=True, check=True)
        match = pattern.search(result.stdout)
        if not match:
            raise ValueError(f"Impossible d'extraire le débit dans la sortie de {bin_path} (run {i}) :\n{result.stdout}")
        val = float(match.group(1))
        values.append(val)
        print(f"  [Run {i}/{runs}] Débit mesuré : {val:.2f} Mops/s")

    return values

def main() -> int:
    parser = argparse.ArgumentParser(description="Vérification de non-régression du banc de performance.")
    parser.add_argument("--baseline", type=Path, default=Path("native/bench/baseline.json"), help="Fichier JSON de référence")
    parser.add_argument("--bin", type=Path, default=Path("bin/bench_math_core"), help="Binaire du banc de mesure")
    parser.add_argument("--runs", type=int, default=None, help="Nombre d'exécutions (défaut: valeur dans baseline.json ou 3)")
    parser.add_argument("--threshold", type=float, default=None, help="Seuil de régression maximum en %% (défaut: valeur dans baseline.json ou 5.0)")
    args = parser.parse_args()

    if not args.baseline.exists():
        print(f"ERREUR : Fichier baseline introuvable : {args.baseline}", file=sys.stderr)
        return 1

    if not args.bin.exists():
        print(f"ERREUR : Binaire de banc introuvable : {args.bin}. Veuillez le compiler avant.", file=sys.stderr)
        return 1

    with open(args.baseline, "r", encoding="utf-8") as f:
        baseline_data = json.load(f)

    baseline_val = float(baseline_data.get("baseline_value", 600.0))
    threshold_pct = float(args.threshold if args.threshold is not None else baseline_data.get("max_regression_percent", 5.0))
    runs = int(args.runs if args.runs is not None else baseline_data.get("runs", 3))

    print(f"=== COMPARAISON DU BANC DE PERFORMANCE ({baseline_data.get('benchmark', 'benchmark')}) ===")
    print(f"Référence attendue : {baseline_val:.2f} Mops/s")
    print(f"Seuil de tolérance de régression : {threshold_pct:.1f} %")
    print(f"Nombre d'itérations de mesure : {runs}")

    try:
        measurements = run_benchmark(args.bin, runs)
    except Exception as e:
        print(f"ERREUR lors de l'exécution du banc : {e}", file=sys.stderr)
        return 1

    med = statistics.median(measurements)
    min_allowed = baseline_val * (1.0 - (threshold_pct / 100.0))
    delta_pct = ((med - baseline_val) / baseline_val) * 100.0

    print(f"\n--- RÉSULTAT STATISTIQUE ---")
    print(f"Médiane mesurée : {med:.2f} Mops/s (Seuil minimal autorisé : {min_allowed:.2f} Mops/s)")
    print(f"Variation par rapport à la baseline : {delta_pct:+.2f} %")

    if med < min_allowed:
        regression_amount = ((baseline_val - med) / baseline_val) * 100.0
        print(f"✗ ÉCHEC : Régression de performance détectée ({regression_amount:.2f}% > seuil de {threshold_pct:.1f}%) !", file=sys.stderr)
        return 1

    print(f"✓ PASS : Débit dans les tolérances (Médiane: {med:.2f} Mops/s >= Min: {min_allowed:.2f} Mops/s).")
    return 0

if __name__ == "__main__":
    sys.exit(main())
