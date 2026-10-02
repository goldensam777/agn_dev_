# Core JS : TypedArrays, ArrayBuffers & Zéro-Copie

> Ce document établit les standards de manipulation binaire haute performance en JavaScript.

---

## 1. ArrayBuffer & Vues Typées

Un `ArrayBuffer` est une plage contiguë d'octets bruts en mémoire machine, analogue à un `malloc` en C :
- Ne jamais manipuler de grands jeux de données numériques avec des tableaux génériques `Array`.
- Utiliser les vues typées appropriées :
  - `Float64Array` : Vecteurs et matrices scientifiques IEEE 754 (8 octets par nombre).
  - `Float32Array` : Données de vertex et buffers WebGL/WebGPU (4 octets).
  - `Uint8Array` : Paquets réseau, cryptographie, buffers binaires.
  - `DataView` : Encodage et décodage avec contrôle explicite du boutisme (*endianness* : Little vs Big Endian).

---

## 2. Transfert de Propriété Zero-Copy (*Transferable Objects*)

Lors de la transmission de données massives vers un Web Worker ou un `worker_thread` Node.js :
- Si vous envoyez le buffer par défaut, le navigateur fait une copie intégrale lente de 100 Mo.
- **La solution Zero-Copy :** Passer le buffer dans la liste de transfert. Le thread expéditeur cède instantanément la propriété au thread récepteur en $O(1)$ :
```javascript
const bigData = new Float64Array(10_000_000); // ~80 Mo

// Le buffer sous-jacent est transféré sans la moindre copie d'octet
worker.postMessage({ type: "PROCESS_DATA", buffer: bigData.buffer }, [bigData.buffer]);

// bigData.buffer.byteLength vaut désormais 0 sur le thread principal (détaché) !
```

---

## 3. `subarray()` vs `slice()`

- `buffer.subarray(start, end)` : Crée une vue sur la même mémoire existante ($O(1)$, zéro allocation).
- `buffer.slice(start, end)` : Alloue un nouveau buffer et copie tous les octets ($O(N)$).
- **Règle :** Toujours privilégier `subarray` pour découper des sous-vecteurs ou des messages réseau.

---

## 4. Checklist Actionnable pour l'Agent

- [ ] Les flux numériques et données scientifiques utilisent-ils `TypedArray` plutôt que `Array` standard ?
- [ ] Les transferts multithreads exploitent-ils la liste de transfert (*transferable list*) ?
- [ ] Les découpages de buffers utilisent-ils `subarray()` pour éviter les copies inutiles ?
