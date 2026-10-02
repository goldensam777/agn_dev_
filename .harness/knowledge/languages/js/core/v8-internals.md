# Core JS : Moteur V8, Classes Cachées & Caches en Ligne

> Ce document détaille le fonctionnement interne du compilateur TurboFan et de l'interpréteur Ignition de V8.

---

## 1. Classes Cachées (*Hidden Classes / Shapes*)

JavaScript n'a pas de classes au sens mémoire C++, mais le moteur V8 crée des structures internes appelées **Shapes** pour mapper les décalages mémoire des propriétés :
- Si deux objets sont instanciés avec les mêmes propriétés dans le même ordre, ils partagent la même Shape.
- Si vous ajoutez une propriété conditionnelle dynamique plus tard, V8 doit créer une transition de Shape coûteuse.

```javascript
// ✓ BON : Même Shape partagée par toutes les instances
class Particle {
  constructor(x, y, vx, vy) {
    this.x = x;
    this.y = y;
    this.vx = vx;
    this.vy = vy;
  }
}

// ❌ MAUVAIS : Crée des Shapes divergentes et brise l'optimisation JIT
const p1 = new Particle(0, 0, 1, 1);
const p2 = new Particle(0, 0, 1, 1);
p1.mass = 10; // Transition de Shape 1
p2.charge = 5; // Transition de Shape 2
```

---

## 2. Inline Caches (IC) : Monomorphisme vs Mégamorphisme

Lorsqu'une fonction accède à une propriété (`obj.x`) :
- **Monomorphique (1 seule Shape observée) :** V8 remplace l'accès par un simple décalage mémoire direct ($O(1)$ ultra-rapide).
- **Polymorphique (2 à 4 Shapes) :** Petite table de vérification en cascade.
- **Mégamorphique (5 Shapes ou plus) :** Abandon de l'optimisation JIT. Rétrogradation en consultation de dictionnaire lent (*slow dictionary lookup*).

---

## 3. Checklist Actionnable pour l'Agent

- [ ] Les propriétés des objets d'une même collection sont-elles toujours initialisées dans le même ordre ?
- [ ] Aucun champ n'est-il ajouté dynamiquement après la construction de l'objet ?
- [ ] Les fonctions chaudes reçoivent-elles toujours des objets de forme identique (*monomorphisme*) ?
