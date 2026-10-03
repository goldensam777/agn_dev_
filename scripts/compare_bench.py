#!/usr/bin/env python3
"""
scripts/compare_bench.py — Comparateur et gestionnaire de baseline de performance multi-machines

Exécute le banc de mesure plusieurs fois, calcule la médiane pour éliminer le bruit,
compare la performance à la machine de référence correspondante, et gère les références
par architecture sous native/bench/baselines/<cpu_slug>.json.
"""

from __future__ import annotations
import argparse
import datetime
import json
import os
import platform
import re
import statistics
import subprocess
import sys
from pathlib import Path

def get_cpu_model() -> str:
    if "FORGE_MOCK_CPU" in os.environ:
        return os.environ["FORGE_MOCK_CPU"]
    try:
        with open("/proc/cpuinfo", "r", encoding="utf-8") as f:
            for line in f:
                if "model name" in line:
                    return line.split(":", 1)[1].strip()
    except Exception:
        pass
    return platform.processor() or "Unknown CPU"

def cpu_to_slug(cpu: str) -> str:
    s = re.sub(r"[^a-zA-Z0-9]+", "_", cpu.lower()).strip("_")
    return s or "unknown_cpu"

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
    parser.add_argument("--baseline", type=Path, default=Path("native/bench/baseline.json"), help="Fichier JSON de référence par défaut")
    parser.add_argument("--baselines-dir", type=Path, default=Path("native/bench/baselines"), help="Répertoire des références multi-machines")
    parser.add_argument("--bin", type=Path, default=Path("bin/bench_math_core"), help="Binaire du banc de mesure")
    parser.add_argument("--runs", type=int, default=None, help="Nombre d'exécutions (défaut: valeur dans baseline ou 5)")
    parser.add_argument("--threshold", type=float, default=None, help="Seuil de régression maximum en %% (défaut: valeur dans baseline ou 5.0)")
    parser.add_argument("--update", action="store_true", help="Met à jour la baseline pour cette machine")
    args = parser.parse_args()

    if not args.bin.exists():
        print(f"ERREUR : Binaire de banc introuvable : {args.bin}. Veuillez le compiler avant via 'make -f native/Makefile bench'.", file=sys.stderr)
        return 1

    current_cpu = get_cpu_model()
    current_slug = cpu_to_slug(current_cpu)
    args.baselines_dir.mkdir(parents=True, exist_ok=True)
    machine_baseline_path = args.baselines_dir / f"{current_slug}.json"

    # Déterminer la baseline à charger : priorité à la baseline spécifique de la machine
    active_baseline_path = args.baseline
    if machine_baseline_path.exists():
        active_baseline_path = machine_baseline_path

    baseline_data: dict = {}
    if active_baseline_path.exists():
        with open(active_baseline_path, "r", encoding="utf-8") as f:
            baseline_data = json.load(f)
    elif not args.update:
        print("====================================================")
        print("⚠ AVERTISSEMENT : BANC NON COMPARABLE")
        print(f"Machine actuelle     : {current_cpu} ({platform.system()} {platform.machine()})")
        print(f"Aucune référence existante dans {machine_baseline_path} ni {args.baseline}.")
        print(f"\nPour enregistrer une référence pour cette machine :")
        print(f"  python3 scripts/compare_bench.py --update --runs 5")
        print("====================================================")
        return 0

    ref_machine = baseline_data.get("reference_machine", {})
    ref_cpu = ref_machine.get("cpu", "")

    # Comparaison de l'architecture matérielle
    if not args.update and ref_cpu and ref_cpu != current_cpu:
        print("====================================================")
        print("⚠ AVERTISSEMENT : BANC NON COMPARABLE")
        print(f"Machine actuelle     : {current_cpu} ({platform.system()} {platform.machine()})")
        print(f"Machine de référence : {ref_cpu}")
        print("\nLe matériel d'exécution diffère de la référence enregistrée.")
        print("Une comparaison de débit entre architectures différentes n'est pas significative.")
        print("\nPour enregistrer une baseline de référence dédiée pour cette machine :")
        print("  python3 scripts/compare_bench.py --update --runs 5")
        print(f"Fichier cible : {machine_baseline_path}")
        print("====================================================")
        return 0

    runs = int(args.runs if args.runs is not None else baseline_data.get("runs", 5))
    threshold_pct = float(args.threshold if args.threshold is not None else baseline_data.get("max_regression_percent", 5.0))
    baseline_val = float(baseline_data.get("baseline_value", 1000.0))

    print(f"=== COMPARAISON DU BANC DE PERFORMANCE ({baseline_data.get('benchmark', 'dot_product')}) ===")
    print(f"Machine actuelle : {current_cpu} ({platform.system()} {platform.machine()})")
    print(f"Référence active : {baseline_val:.2f} Mops/s ({active_baseline_path})")
    print(f"Seuil de tolérance : {threshold_pct:.1f} %")
    print(f"Nombre d'itérations : {runs}")

    try:
        measurements = run_benchmark(args.bin, runs)
    except Exception as e:
        print(f"ERREUR lors de l'exécution du banc : {e}", file=sys.stderr)
        return 1

    med = statistics.median(measurements)
    print(f"\n--- RÉSULTAT STATISTIQUE ---")
    print(f"Médiane calculée : {med:.2f} Mops/s")

    if args.update:
        new_data = {
            "benchmark": "dot_product",
            "metric": "mops",
            "baseline_value": round(med, 2),
            "unit": "Millions d'opérations/sec",
            "max_regression_percent": threshold_pct,
            "runs": runs,
            "reference_machine": {
                "cpu": current_cpu,
                "cores": subprocess.check_output(["nproc"]).decode().strip(),
                "os": platform.platform(),
                "architecture": platform.machine(),
            },
            "last_updated": datetime.datetime.now().isoformat(),
            "description": "Banc de performance Mercuria dot_product (N = 10,000,000 éléments)",
            "update_procedure": f"Pour actualiser cette machine : python3 scripts/compare_bench.py --update --runs {runs}",
        }
        with open(machine_baseline_path, "w", encoding="utf-8") as f:
            json.dump(new_data, f, indent=2)
        with open(args.baseline, "w", encoding="utf-8") as f:
            json.dump(new_data, f, indent=2)
        print(f"✓ Baseline enregistrée dans {machine_baseline_path} et {args.baseline} ({med:.2f} Mops/s).")
        return 0

    min_allowed = baseline_val * (1.0 - (threshold_pct / 100.0))
    delta_pct = ((med - baseline_val) / baseline_val) * 100.0
    print(f"Seuil minimal autorisé : {min_allowed:.2f} Mops/s")
    print(f"Variation par rapport à la baseline : {delta_pct:+.2f} %")

    if med < min_allowed:
        regression_amount = ((baseline_val - med) / baseline_val) * 100.0
        print(f"✗ ÉCHEC : Régression de performance détectée ({regression_amount:.2f}% > seuil de {threshold_pct:.1f}%) !", file=sys.stderr)
        return 1

    print(f"✓ PASS : Débit dans les tolérances (Médiane: {med:.2f} Mops/s >= Min: {min_allowed:.2f} Mops/s).")
    return 0

if __name__ == "__main__":
    sys.exit(main())
