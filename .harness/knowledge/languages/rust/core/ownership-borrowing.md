# Core Rust : Propriété, Emprunt & Lifetimes

> Ce document établit les règles d'or de la gestion de mémoire et du système d'emprunt en Rust. Lu pour toute tâche Rust.

---

## 1. Le Théorème Fondamental : Aliasing XOR Mutabilité

En Rust, l'inviolabilité mémoire repose sur une règle binaire absolue :
- Soit un nombre quelconque de lecteurs simultanés (`&T`).
- Soit un unique écrivain exclusif (`&mut T`).
- **JAMAIS les deux en même temps pour une même case mémoire.**

Toute tentative de contourner cette règle par de la mutabilité intérieure (`RefCell`, `Mutex`) doit être justifiée par un besoin architectural réel et non par la paresse d'ajuster une lifetime.

---

## 2. Table de Décision pour le Passage de Paramètres

| Intention | Type recommandé | Pourquoi |
|---|---|---|
| Lecture seule sans transfert de propriété | `&T` (ou `&str`, `&[T]`) | Zero-cost, n'alloue rien, n'impose pas la possession. |
| Modification exclusive en place | `&mut T` | Garantit l'absence d'aliasing concurrent. |
| Consommation / Transformation de propriété | `T` (par valeur) | Déplace la valeur (*move semantics*), libérée à la sortie. |
| Lecture flexible (propriété ou référence) | `Cow<'a, B>` | Clone uniquement si une écriture/mutation survient (*Copy-on-Write*). |
| Paramètre générique de chemin de fichier | `impl AsRef<Path>` | Accepte `&str`, `String`, `&Path`, `PathBuf` sans allocation intermédiaire. |
| Paramètre de texte abstrait | `impl AsRef<str>` | Évite d'obliger l'appelant à allouer une `String`. |

---

## 3. Gestion des Lifetimes & Règles d'Élision

1. **Laisser le compilateur élider quand c'est possible :**
   - Ne pas encombrer les signatures de `<'a>` inutiles si les règles d'élision standards suffisent.
   - Exemple : `fn first_word(s: &str) -> &str` est automatiquement compris comme `fn first_word<'a>(s: &'a str) -> &'a str`.
2. **Lifetimes explicites lors de retours multi-arguments :**
   - Dès qu'une fonction prend plusieurs références en entrée et en renvoie une, expliciter d'où provient la référence retournée.
3. **Interdiction absolue des structures auto-référentielles naïves :**
   - En Rust, une `struct Node<'a> { parent: Option<&'a Node<'a>> }` mène directement à l'impasse du borrow checker.
   - **Remède architectural :** Utiliser des arènes avec index (`NodeId(u32)` ou `bumpalo`).

---

## 4. `Clone` vs `Copy` : Règle de Performance

- **Ne jamais dériver `Clone` pour masquer un problème d'emprunt.**
- Un appel à `.clone()` sur une structure volumineuse (vecteur, chaîne, AST) au milieu d'une boucle critique est une faute de conception majeure.
- Dériver `Copy` uniquement pour les types de taille triviale (généralement $\le 16$ octets, types scalaires ou petits tuples de scalaires) ne gérant aucune ressource heap.

---

## 5. Checklist Actionnable pour l'Agent

- [ ] Les paramètres en lecture seule prennent-ils `&str` / `&[T]` plutôt que `&String` / `&Vec<T>` ?
- [ ] Aucun `.clone()` n'est-il appelé pour calmer le borrow checker sans justification ?
- [ ] Les structures contenant des références ont-elles des lifetimes bien contraintes ?
- [ ] Aucun cycle de références fortes (`Rc<RefCell<T>>`) risquant de créer une fuite mémoire ?
