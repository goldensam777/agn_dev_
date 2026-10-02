# 05. Environnements d'Exécution & Gestion Mémoire Runtime

> Synthèse algorithmique issue du Dragon Book (Chapitre 7 : *Run-Time Environments*). Cadres d'activation, fermetures lexicales et ramasse-miettes (Garbage Collection).

---

## 1. Organisation de l'Espace d'Adressage Mémoire

La mémoire d'un programme compilé est structurée en 4 segments principaux :
1. **Code & Données Statiques :** Instructions machines en lecture seule, constantes globales, tables virtuelles (`vtable`).
2. **Pile d'Exécution (Call Stack) :** Allocation et désallocation déterministes LIFO des cadres d'activation.
3. **Espace Libre :** Zone d'expansion entre la pile (qui croît vers les adresses basses) et le tas (vers les adresses hautes).
4. **Tas (Heap) :** Allocations dynamiques dont la durée de vie outrepasse l'appel de fonction d'origine.

---

## 2. Anatomie d'un Cadre d'Activation (Stack Frame)

Un enregistrement d'activation standard contient :
- **Paramètres réels :** Transmis par l'appelant (souvent dans les registres sous x86-64 System V ABI, débordement sur pile).
- **Lien de Contrôle (Dynamic Link) :** Pointeur vers le cadre d'activation de la fonction appelante (restaure le `rbp` précédent).
- **Lien d'Accès (Static Link / Access Link) :** Pointeur vers le cadre de la fonction englobante lexicale la plus proche (indispensable pour les fonctions imbriquées et les fermetures lexicales / closures).
- **Adresse de retour :** Pointeur d'instruction sauvegardé (`rip`) où reprendre l'exécution après le retour.
- **Variables locales et temporaires :** Scalaires et structures locales de taille connue à la compilation.

---

## 3. Gestion du Tas & Fragmentation

- **Fragmentation Externe :** La mémoire libre totale est suffisante pour satisfaire une requête, mais aucun bloc contigu individuel n'est assez grand.
- **Algorithmes de Placement Libre :**
  - *First-Fit :* Alloue le premier bloc suffisant (très rapide).
  - *Best-Fit :* Alloue le plus petit bloc suffisant pour minimiser l'espace perdu (réduit la fragmentation externe moyenne).
  - *Buddy System :* Divise récursivement des blocs de taille $2^k$ (fusion $O(1)$ ultra-rapide lors de la libération).

---

## 4. Algorithmes Canoniques de Ramasse-Miettes (GC)

### A. Mark-Sweep (Traçage Deux Phases) :
1. **Phase de Marquage (Mark) :** Depuis l'ensemble des racines (pile, registres, variables globales), parcourir tous les objets atteignables et positionner leur bit de marquage à 1.
2. **Phase de Balayage (Sweep) :** Parcourir séquentiellement l'intégralité du tas ; si un bloc a son bit à 0, l'ajouter à la liste libre ; si son bit est à 1, le réinitialiser à 0 pour le cycle suivant.
- *Complexité :* $O(\text{taille totale du tas})$.

### B. Collecteur Copiant de Cheney (Stop-and-Copy) :
Divise le tas en deux demi-espaces : `FromSpace` et `ToSpace`.
1. Lors de la collecte, les objets vivants sont copiés sélectivement de `FromSpace` vers `ToSpace`.
2. Les références internes sont mises à jour via des pointeurs de redirection (*forwarding pointers*).
3. À la fin, les rôles de `FromSpace` et `ToSpace` sont inversés.
- *Avantages majeurs :*
  - Complexité en $O(\text{volume des données vivantes})$ — indépendant de la mémoire morte.
  - **Compactage automatique :** Élimination totale de la fragmentation externe.
  - Allocation en $O(1)$ par simple incrément de pointeur de frontière (*bump allocation*).
