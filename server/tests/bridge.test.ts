import { test, describe } from "node:test";
import assert from "node:assert/strict";
import { NativeBridge } from "../src/bridge/native_bridge.js";
import path from "node:path";

describe("NativeBridge & Moteur C++", () => {
  const binaryPath = path.resolve(process.cwd(), "bin/forge_compute");
  const bridge = new NativeBridge(binaryPath);

  test("le bridge détecte un moteur natif sain", async () => {
    const healthy = await bridge.isHealthy();
    assert.strictEqual(healthy, true, "Le moteur natif C++ devrait être opérationnel");
  });

  test("calcul Monte Carlo Pi via le bridge natif", async () => {
    const res = await bridge.executeJob({
      jobId: "11111111-1111-1111-1111-111111111111",
      algorithm: "monte_carlo_pi",
      dimensions: 100,
      iterations: 200_000,
    });

    assert.strictEqual(res.status, "completed");
    assert.ok(res.summaryValue > 3.10 && res.summaryValue < 3.20, `Valeur de Pi incohérente : ${res.summaryValue}`);
    assert.ok(res.executionTimeMs >= 0);
    assert.ok(res.resultChecksum.startsWith("chk_"));
  });

  test("calcul Produit Scalaire via le bridge natif", async () => {
    const res = await bridge.executeJob({
      jobId: "22222222-2222-2222-2222-222222222222",
      algorithm: "vector_dot_product",
      dimensions: 50_000,
      iterations: 10,
    });

    assert.strictEqual(res.status, "completed");
    assert.ok(res.summaryValue > 0);
    assert.ok(res.executionTimeMs < 100, "Le calcul SIMD devrait prendre moins de 100ms");
  });
});
