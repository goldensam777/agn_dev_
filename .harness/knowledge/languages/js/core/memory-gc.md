# Core JS : Gestion Mémoire & Garbage Collector V8

> Ce document fixe les règles de réduction de la pression sur le ramasse-miettes (*GC pressure*) et d'élimination des fuites mémoire.

---

## 1. Les Deux Mondes du GC V8

V8 applique l'hypothèse générationnelle : la plupart des objets meurent très jeunes.
1. **Young Generation (Scavenger) :** Nettoie les allocations éphémères très rapidement.
2. **Old Generation (Major GC) :** Objets ayant survécu à deux cycles. Le nettoyage par *Mark-Sweep-Compact* est coûteux et peut provoquer des микро-gels d'exécution (*GC pauses*).

**Règle d'or :** Minimiser le taux d'allocation dans les boucles de calcul (ne pas instancier d'objets ou de fermetures jetables à chaque itération).

---

## 2. Fuites Mémoires par Fermetures Implicites (*Closures*)

Une fonction interne retient en mémoire tout le contexte lexical de sa fonction parente, même les variables non utilisées directement :

```javascript
// ❌ FUITE MÉMOIRE SUBTILE : bigData est retenu indéfiniment en mémoire par le timer
function startTracking() {
  const bigData = new Uint8Array(50 * 1024 * 1024); // 50 Mo
  const id = 42;

  return setInterval(() => {
    // Bien que ce closure n'utilise que 'id', V8 peut retenir tout le scope parent contenant bigData !
    console.log("Ping", id);
  }, 1000);
}
```

---

## 3. Usage Impératif de `WeakMap` et `WeakSet`

Pour associer des métadonnées à des objets sans empêcher leur libération par le GC :
- Utiliser systématiquement `WeakMap` plutôt que `Map`. Dès que l'objet clé n'est plus référencé nulle part, l'entrée s'évapore automatiquement sans fuite mémoire.

---

## 4. Checklist Actionnable pour l'Agent

- [ ] Les caches et registres d'objets utilisent-ils `WeakMap` / `WeakSet` pour éviter d'empêcher le GC ?
- [ ] Les boucles critiques réutilisent-elles des objets préalloués au lieu d'en instancier de nouveaux ?
- [ ] Aucun timer (`setInterval`) n'omet-il son `clearInterval` ?
