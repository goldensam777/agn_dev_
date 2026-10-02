# Domaine JS : Flux Continus (Streams) & Gestion de la Contre-Pression

> Ce document établit le traitement de données volumineuses par flux sans explosion de mémoire vive.

---

## 1. La Règle d'Or : Respecter la Contre-Pression (*Backpressure*)

Ne jamais charger un fichier ou une réponse réseau de plusieurs gigaoctets entièrement en mémoire vive :
- Utiliser les flux (*Streams*) et relier la source à la destination avec `pipeline` (Node.js) ou `.pipeThrough()` (Web Streams) :
```javascript
import { pipeline } from "node:stream/promises";
import fs from "node:fs";
import zlib from "node:zlib";

// La mémoire tampon reste bornée à ~64 Ko quel que soit le volume du fichier
await pipeline(
  fs.createReadStream("dataset_geante.csv"),
  zlib.createGzip(),
  fs.createWriteStream("dataset_geante.csv.gz")
);
```

---

## 2. Checklist Actionnable pour l'Agent

- [ ] Tout transfert ou transformation de fichier massif utilise-t-il les Streams avec `pipeline()` ?
- [ ] Aucun appel à `fs.readFileSync()` n'est utilisé sur des fichiers volumineux non bornés ?
- [ ] La contre-pression est-elle respectée lors de l'écriture dans des sockets réseau ?
