#!/usr/bin/env python3
"""
scripts/query_book.py — Outil d'interrogation chirurgicale des ouvrages de référence

Permet aux agents d'IA et aux développeurs d'extraire des extraits précis,
rechercher des algorithmes ou consulter des pages de livres de référence sans
charger des mégaoctets de PDF dans le contexte LLM.
"""

from __future__ import annotations
import argparse
import subprocess
import sys
from pathlib import Path

ROOT_DIR = Path(__file__).resolve().parent.parent
DOCS_DIR = ROOT_DIR / "docs" / "references"

BOOKS: dict[str, Path] = {
    "dragon": DOCS_DIR / "dragon_book.pdf",
    "crafting": DOCS_DIR / "crafting_interpreters.pdf",
}

def extract_pages(pdf_path: Path, first_page: int, last_page: int) -> str:
    """Extrait le texte brut d'une plage de pages via pdftotext."""
    if not pdf_path.exists():
        raise FileNotFoundError(f"Livre introuvable à l'emplacement : {pdf_path}")

    cmd = [
        "pdftotext",
        "-f", str(first_page),
        "-l", str(last_page),
        "-layout",
        str(pdf_path),
        "-",
    ]
    result = subprocess.run(cmd, capture_output=True, text=True, check=True)
    return result.stdout

def search_keyword(pdf_path: Path, query: str, max_results: int = 10) -> list[tuple[int, str]]:
    """Recherche un mot-clé ou une regex dans l'ensemble du livre page par page."""
    if not pdf_path.exists():
        raise FileNotFoundError(f"Livre introuvable à l'emplacement : {pdf_path}")

    # Extraction globale ou recherche rapide
    # Utilisation de pdftotext pour extraire le texte et localiser les occurrences
    cmd = ["pdftotext", str(pdf_path), "-"]
    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, text=True, errors="replace")

    results: list[tuple[int, str]] = []
    current_page = 1
    current_page_text: list[str] = []

    if proc.stdout is None:
        return results

    query_lower = query.lower()

    for line in proc.stdout:
        if "\x0c" in line:  # Form feed character = changement de page dans pdftotext
            page_content = "\n".join(current_page_text)
            if query_lower in page_content.lower():
                # Trouver la ligne spécifique
                matching_lines = [
                    l.strip() for l in current_page_text if query_lower in l.lower()
                ]
                snippet = " | ".join(matching_lines[:2])
                results.append((current_page, snippet[:200]))
                if len(results) >= max_results:
                    break
            current_page += 1
            current_page_text = []
        else:
            current_page_text.append(line.rstrip())

    proc.terminate()
    return results

def main() -> int:
    parser = argparse.ArgumentParser(
        description="Recherche chirurgicale dans la bibliothèque d'ouvrages théoriques de la Forge."
    )
    parser.add_argument(
        "--book",
        choices=["dragon", "crafting"],
        default="dragon",
        help="Livre à interroger (défaut: dragon)",
    )
    parser.add_argument(
        "--page",
        type=int,
        help="Numéro de page exacte à extraire",
    )
    parser.add_argument(
        "--count",
        type=int,
        default=1,
        help="Nombre de pages consécutives à extraire (défaut: 1)",
    )
    parser.add_argument(
        "--search",
        type=str,
        help="Terme ou mot-clé à rechercher dans le livre",
    )
    parser.add_argument(
        "--max-results",
        type=int,
        default=5,
        help="Nombre maximum de résultats de recherche (défaut: 5)",
    )

    args = parser.parse_args()
    pdf_path = BOOKS.get(args.book)
    if not pdf_path or not pdf_path.exists():
        print(f"Erreur : Le fichier pour '{args.book}' n'existe pas : {pdf_path}", file=sys.stderr)
        return 1

    if args.page is not None:
        last_page = args.page + max(1, args.count) - 1
        print(f"=== Extraction de [{args.book}] — Pages {args.page} à {last_page} ===\n")
        try:
            content = extract_pages(pdf_path, args.page, last_page)
            print(content)
            return 0
        except Exception as e:
            print(f"Erreur d'extraction : {e}", file=sys.stderr)
            return 1

    if args.search:
        print(f"=== Recherche de '{args.search}' dans [{args.book}] (max: {args.max_results}) ===\n")
        try:
            matches = search_keyword(pdf_path, args.search, args.max_results)
            if not matches:
                print("Aucune occurrence trouvée.")
                return 0
            for page, snippet in matches:
                print(f"- Page {page} : {snippet}")
            return 0
        except Exception as e:
            print(f"Erreur lors de la recherche : {e}", file=sys.stderr)
            return 1

    parser.print_help()
    return 0

if __name__ == "__main__":
    sys.exit(main())
