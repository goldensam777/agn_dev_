# Domaine Scientifique : Calcul & Algorithmes en Production

> Base de connaissances transverse pour la rigueur numérique, la reproductibilité déterministe et l'exploitation des calculs scientifiques lourds en production.

---

## 1. Organisation du Domaine

| Fichier | Sujet Traité |
|---|---|
| [`scientific-in-production.md`](scientific-in-production.md) | Reproductibilité absolue, traçabilité de provenance, sécurité numérique aux frontières (NaN/Inf), tolérances relatives et absolues, orchestration des jobs lourds et monitoring du drift scientifique. |

---

## 2. La Règle d'Or du Calcul Scientifique de la Forge

> **« Un résultat presque correct est une réponse fausse avec de bonnes manières. »**
> 
> En calcul scientifique et en ingénierie, une approximation instable ou un arrondi cumulatif non maîtrisé invalide l'ensemble de la chaîne décisionnelle. Tout calcul produit doit être accompagné de son intervalle de confiance, de sa tolérance d'erreur mesurée et de son empreinte cryptographique de reproductibilité.
