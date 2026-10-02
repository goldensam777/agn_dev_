/**
 * @file canonical_worker_pipeline.worker.js
 * @brief Partie worker du pipeline : transformation PURE, aucun état partagé.
 *
 * Règles (corpus js/) : le worker ne stocke RIEN entre les messages (pas de fuite
 * inter-requêtes), ne loggue pas les données (volume), et renvoie par transfert.
 */

import { parentPort } from "node:worker_threads";

if (parentPort === null) {
  throw new Error("Ce module doit être lancé comme worker");
}

parentPort.on("message", (message) => {
  const { chunk } = message;
  const input = new Float64Array(chunk); // vue zéro-copie sur le buffer transféré
  const out = new ArrayBuffer(chunk.byteLength);
  const result = new Float64Array(out);
  for (let i = 0; i < input.length; i += 1) {
    result[i] = Math.hypot(input[i], input[i]); // exemple de transformation CPU-bound
  }
  parentPort.postMessage(out, [out]); // TRANSFER retour : zéro copie
});
