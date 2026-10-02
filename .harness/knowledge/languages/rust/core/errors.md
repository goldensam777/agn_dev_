# Core Rust : Gestion des Erreurs & Politique de Panique

> Ce document fixe la stratégie de gestion d'erreurs en Rust. Aucun crash silencieux ni panique sauvage n'est toléré.

---

## 1. La Dichotomie Fondamentale : `thiserror` vs `anyhow`

En Rust professionnel, le choix du gestionnaire d'erreur dépend strictement de la couche applicative :

| Couche logicielle | Bibliothèque / Outil | Justification |
|---|---|---|
| **Bibliothèque / Moteur de calcul / Parseur (`native/`)** | **`thiserror`** | Doit exposer des énumérations d'erreurs typées, exhaustives et inspectables par pattern matching. Pas d'effacement de type (`type erasure`). |
| **Orchestrateur / CLI / Serveur (`server/`)** | **`anyhow`** | Reçoit les erreurs de plusieurs sous-systèmes, ajoute du contexte dynamique via `.context("...")` et formate la trace d'erreur finale. |

---

## 2. Politique de Panique (`panic!`, `unwrap`, `expect`)

1. **Interdiction de `unwrap()` en production :**
   - L'appel à `.unwrap()` est strictement rejeté en revue de code sur tout composant critique.
2. **Usage très encadré de `expect("...")` :**
   - Un `.expect("message")` n'est toléré que si l'erreur relève d'une **impossibilité logique prouvée** que le compilateur ne peut pas déduire.
   - Le message doit obligatoirement expliquer **pourquoi** l'état est garanti valide :
     ```rust
     // ❌ MAUVAIS
     let item = list.first().expect("empty");

     // ✓ BON (Prouve l'invariant)
     assert!(!list.is_empty(), "La liste a été filtrée non vide à la ligne précédente");
     let item = list.first().expect("Invariant garanti par le test is_empty");
     ```
3. **Quand utiliser `panic!` ou `assert!` ? :**
   - Exclusivement pour les **violations d'invariants internes du programme** (bugs de programmation, corruption mémoire, dépassement de bornes impossible).
   - Jamais pour des données entrantes invalides (qui doivent renvoyer `Result::Err`).

---

## 3. Propagation Élégante avec l'Opérateur `?`

- Toujours utiliser `?` pour propager les erreurs.
- Pour enrichir une erreur avec du contexte opérationnel sans perdre la cause d'origine :
  ```rust
  let content = fs::read_to_string(path)
      .with_context(|| format!("Impossible de lire le fichier de configuration : {}", path.display()))?;
  ```

---

## 4. Checklist Actionnable pour l'Agent

- [ ] Aucun `.unwrap()` présent dans le code de bibliothèque / moteur ?
- [ ] Les erreurs des modules internes sont-elles définies via un `enum MyError` avec `#[derive(thiserror::Error)]` ?
- [ ] Les `expect()` fournissent-ils une explication de l'invariant logique et non une simple répétition de l'action ?
- [ ] Les erreurs I/O ou réseau propagent-elles le chemin ou l'URL incriminé via du contexte ?
