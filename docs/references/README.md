# Bibliothèque de Références Théoriques & Ouvrages Fondateurs

Ce répertoire héberge les ouvrages académiques et industriels de référence utilisés par les agents de la Forge et l'équipe humaine.

---

## 1. Ouvrages Présents

| Fichier | Titre | Auteurs | Thématique |
|---|---|---|---|
| `dragon_book.pdf` | *Compilers: Principles, Techniques, & Tools* (2nd Edition, 2007) | Alfred V. Aho, Monica S. Lam, Ravi Sethi, Jeffrey D. Ullman | Théorie des automates, parsing, SDD, TAC, SSA, optimisations, allocation de registres. |
| `crafting_interpreters.pdf` | *Crafting Interpreters* (2021) | Robert Nystrom | Implémentation d'un interpréteur arborescent (jlox) et d'une machine virtuelle à pile avec bytecode (clox). |

---

## 2. Accès & Exploitation par les Agents

1. **Corpus Distillé :** Les algorithmes du Dragon Book sont synthétisés sous forme de fiches techniques ultra-denses dans [`.harness/knowledge/domain/compilers/`](../../.harness/knowledge/domain/compilers/).
2. **Recherche Dynamique :** Pour interroger ponctuellement un extrait ou un numéro de page précis du livre sans saturer le contexte mémoire de l'agent, utiliser le script :
   ```bash
   python3 scripts/query_book.py --book dragon --search "Chaitin"
   python3 scripts/query_book.py --book dragon --page 540 --count 2
   ```
