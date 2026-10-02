/**
 * @file canonical_worker_pipeline.js
 * @brief Modèle canonique ES2024 : pool de workers zéro-copie avec backpressure.
 *
 * Règles illustrées (corpus .harness/knowledge/languages/js/) :
 *  - core/typed-arrays-buffers.md         : les gros buffers voyagent en TRANSFER
 *    (zéro copie) ; après transfert le buffer émetteur est vidé — ne plus l'utiliser.
 *  - domains/workers-multithreading.md    : pool BORNÉ au nombre de cœurs ; jamais
 *    un worker par tâche.
 *  - core/event-loop-libuv.md             : backpressure — produire plus vite que
 *    l'aval ne consomme n'est PAS de la performance, c'est de la latence en dépôt.
 *  - core/event-loop-libuv.md (listeners) : UN handler permanent par worker, jamais
 *    d'accumulation de once() — un EventEmitter n'est pas une file de listeners.
 */

import { cpus } from "node:os";
import path from "node:path";
import { Worker } from "node:worker_threads";
import { pathToFileURL } from "node:url";

export class PipelinePool {
  /** @type {Worker[]} */
  #idle;
  /** @type {Worker[]} */
  #all;
  /** @type {Array<(w: Worker) => void>} */
  #waiters;
  /** @type {Map<Worker, {resolve: (r: ArrayBuffer) => void, reject: (e: Error) => void}>} */
  #pending;

  /**
   * @param {string | URL} workerScript chemin du worker
   * @param {number} [size] taille du pool (défaut : cœurs logiques)
   */
  constructor(workerScript, size = cpus().length) {
    this.#waiters = [];
    this.#pending = new Map();
    this.#all = Array.from({ length: size }, () => {
      const worker = new Worker(workerScript);
      // Listeners PERMANENTS, posés une fois : un worker ne porte jamais plus
      // d'une requête en vol, donc un seul handler 'message'/'error' suffit.
      worker.on("message", (result) => this.#settle(worker, null, result));
      worker.on("error", (err) => this.#settle(worker, err, null));
      return worker;
    });
    this.#idle = [...this.#all];
  }

  /**
   * Soumet un chunk au pipeline.
   * Le buffer est TRANSFÉRÉ : après l'appel, `chunk.byteLength === 0` côté
   * appelant. Le résultat revient lui aussi transféré (zéro copie dans les deux sens).
   *
   * @param {ArrayBuffer} chunk
   * @returns {Promise<ArrayBuffer>}
   */
  async process(chunk) {
    const worker = await this.#acquire(); // backpressure : on attend si le pool est plein
    return new Promise((resolve, reject) => {
      this.#pending.set(worker, { resolve, reject });
      worker.postMessage({ chunk }, [chunk]); // TRANSFER, pas de structured clone
    });
  }

  async close() {
    await Promise.all(this.#all.map((w) => w.terminate()));
  }

  /**
   * @param {Worker} worker
   * @param {Error | null} err
   * @param {ArrayBuffer | null} result
   */
  #settle(worker, err, result) {
    const pending = this.#pending.get(worker);
    if (pending === undefined) return; // message inattendu : ignoré proprement
    this.#pending.delete(worker);
    this.#release(worker); // le worker retourne au pool, même en cas d'erreur
    if (err !== null) pending.reject(err);
    else pending.resolve(/** @type {ArrayBuffer} */ (result));
  }

  /** @returns {Promise<Worker>} */
  #acquire() {
    const worker = this.#idle.pop();
    if (worker !== undefined) return Promise.resolve(worker);
    return new Promise((resolve) => {
      this.#waiters.push(resolve);
    });
  }

  /** @param {Worker} worker */
  #release(worker) {
    const waiter = this.#waiters.shift();
    if (waiter !== undefined) waiter(worker);
    else this.#idle.push(worker);
  }
}

// --- Démonstration directe : node canonical_worker_pipeline.js -------------

const isMain =
  process.argv[1] !== undefined &&
  import.meta.url === pathToFileURL(path.resolve(process.argv[1])).href;

if (isMain) {
  const pool = new PipelinePool(
    new URL("./canonical_worker_pipeline.worker.js", import.meta.url),
    4,
  );
  const results = await Promise.all(
    Array.from({ length: 16 }, (_, i) => {
      const chunk = new Float64Array([i, i + 1, i + 2]).buffer;
      return pool.process(chunk);
    }),
  );
  console.log("Premiers résultats :", results.map((r) => new Float64Array(r)[0]));
  await pool.close();
}
