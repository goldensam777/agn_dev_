# CONVENTIONS.md — Les Règles d'Or de la Forge

Ce fichier contient les règles non négociables appliquées à l'ensemble du code de ce dépôt. Tout code qui enfreint ces règles sera rejeté par le juge de vérification.

---

## 1. Philosophie Générale

1. **Zéro tolérance pour les avertissements :** Tout warning du compilateur ou du linter est traité comme une erreur bloquante (`-Werror`, `clippy -D warnings`, `eslint --max-warnings 0`).
2. **Mémoire irréprochable :** Aucune fuite de mémoire, aucun débordement de tampon, aucun comportement indéterminé (*Undefined Behavior*). Tout code natif doit passer sans faute sous `AddressSanitizer` et `UBSan`.
3. **Typage strict et vérifié aux frontières :** Tout échange de données entre sous-systèmes (Web <-> Serveur <-> Natif) doit être validé à l'exécution par des schémas immuables (`contracts/`).
4. **Performance mesurée, jamais présumée :** Toute optimisation doit être adossée à un benchmark reproductible (pas d'optimisation prématurée à l'aveugle).

---

## 2. Politique Rust-first pour les modules natifs

Rust est le langage par défaut pour tout nouveau module natif ou toute nouvelle
fonctionnalité système.

Le C++ est autorisé uniquement lorsqu'une justification technique documentée
existe, notamment :

- une bibliothèque ou un SDK nécessaire n'a pas d'équivalent Rust viable ;
- l'intégration directe d'une base de code C++ existante est requise ;
- une contrainte d'ABI, de plateforme, de matériel ou d'outillage l'impose ;
- une preuve reproductible montre que Rust ne satisfait pas les exigences de
  performance ou d'intégration.

Avant d'introduire du nouveau C++, il faut vérifier si la dépendance peut être
encapsulée derrière une API C ou une frontière FFI stable, en conservant le
nouveau code applicatif en Rust lorsque cela est possible.

Toute exception doit être documentée dans un ADR ou dans la documentation du
module, avec la dépendance concernée, la frontière FFI et la stratégie de test.

---

## 3. Règles par Langage

### C & C++ (`native/`)

- **Standards :** C23 pour le C, C++20 minimum pour le C++.
- **Gestion mémoire :**
  - En C++ : Interdiction des `new` et `delete` nus. Utiliser exclusivement le RAII, `std::unique_ptr`, `std::shared_ptr`, ou des conteneurs standards (`std::vector`).
  - En C : Toute fonction allouant de la mémoire doit avoir son équivalent de libération explicite documenté.
- **Flags de compilation obligatoires :**

  ```bash
  -Wall -Wextra -Wpedantic -Wconversion -Werror -fsanitize=address,undefined
  ```

- **Conception :** Préférer le *Data-Oriented Design* pour les boucles de calcul scientifique intensif (mise en cache L1/L2/L3, vecteurs contigus).

### Rust (`native/` ou sous-systèmes)

- **Édition :** Rust 2021 ou 2024.
- **Linter :** `cargo clippy -- -D warnings`.
- **Blocs `unsafe` :** Strictement limités aux optimisations extrêmes ou aux appels FFI. Tout bloc `unsafe` doit comporter un commentaire `// SAFETY:` expliquant rigoureusement pourquoi il est sain, et doit passer sous `cargo miri test`.

### TypeScript & React (`web/` & `server/`)

- **Configuration TS :** `"strict": true`, `"noImplicitAny": true`, `"exactOptionalPropertyTypes": true`.
- **Frontières réseau / IPC :** Toujours valider avec `Zod` (dossier `contracts/`). Ne jamais faire confiance aveuglement aux données entrantes avec un simple `as Type`.
- **Composants React :** Composants fonctionnels purs avec typage strict des `props`. Séparation nette entre logique métier (hooks) et rendu d'interface.
- **Rendu scientifique :** Les composants manipulant des flux volumineux (graphes, 3D, simulation) doivent exploiter WebGL ou WebGPU via des canvases isolés.

### Python (Scripts & Analyse scientifique)

- **Typage :** Annotations de types strictes sur toutes les fonctions publiques.
- **Calcul scientifique :** Préférer les opérations vectorisées (`numpy`, `jax`, `scipy`) ou l'appel direct à nos bibliothèques natives compilées (via `nanobind` ou `PyO3`).

---

## 4. Format des Commits & Documentation

- **Format des commits :** Conventional Commits obligatoire (`feat:`, `fix:`, `perf:`, `refactor:`, `test:`, `docs:`).
- **Documentation du code :** Chaque structure de données, classe ou fonction publique doit être documentée (Doxygen pour C++, Rustdoc pour Rust, TSDoc pour TypeScript).

---

## 5. Commande Unique de Vérification

Avant de valider ou soumettre toute modification, la commande suivante doit être exécutée avec succès :

```bash
bash scripts/verify.sh
```
