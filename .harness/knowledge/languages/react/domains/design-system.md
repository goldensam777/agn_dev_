# Domaine React : Design System, Hiérarchie Visuelle & Dashboards Scientifiques

> Ce document fixe les règles de qualité visuelle et d'ergonomie pour les interfaces de laboratoire.

---

## 1. Principes de Design d'une Interface Scientifique

1. **Hiérarchie Visuelle & Lisibilité :**
   - Les métriques clés (temps d'exécution, valeur calculée, statut moteur) doivent être immédiatement repérables d'un coup d'œil.
   - Les codes couleur doivent être universels : Vert pour nominal/en ligne, Rouge pour erreur/panne, Ambre pour dégradé/calcul en cours.
2. **Typographie des Données :**
   - Utiliser des polices à espacement fixe (*monospace*) pour les valeurs numériques, les checksums et les adresses hexadécimales afin d'éviter les sauts de ligne lors des mises à jour.
3. **Gestion des Espacements & Contraste :**
   - Utiliser une échelle d'espacement standardisée (base 4px ou 8px : 4, 8, 16, 24, 32px).
   - Ratio de contraste minimal de 4.5:1 pour le texte normal afin de respecter les critères WCAG AA.

---

## 2. Checklist Actionnable pour l'Agent

- [ ] Les métriques numériques dynamiques utilisent-elles des polices monospace ?
- [ ] Les états critiques (chargement, succès, erreur) sont-ils visuellement distincts sans ambiguïté ?
- [ ] L'interface est-elle responsive et utilisable sans défilement horizontal cassé ?
