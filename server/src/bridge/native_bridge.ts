import { execFile } from "node:child_process";
import { promisify } from "node:util";
import path from "node:path";
import {
  ComputeJobRequest,
  ComputeJobRequestSchema,
  ComputeJobResponse,
  ComputeJobResponseSchema,
} from "@forge/contracts";

import { fileURLToPath } from "node:url";
import fs from "node:fs";

const execFileAsync = promisify(execFile);

/**
 * server/src/bridge/native_bridge.ts
 * Le seul et unique point d'accès au moteur de calcul C++ (native/bin/forge_compute).
 */
export class NativeBridge {
  private binaryPath: string;

  constructor(customBinaryPath?: string) {
    if (customBinaryPath && fs.existsSync(customBinaryPath)) {
      this.binaryPath = customBinaryPath;
      return;
    }

    // Résolution relative au fichier source
    const currentDir = path.dirname(fileURLToPath(import.meta.url));
    const candidatePaths = [
      path.resolve(currentDir, "../../../bin/forge_compute"),
      path.resolve(process.cwd(), "bin/forge_compute"),
      path.resolve(process.cwd(), "../bin/forge_compute"),
    ];

    const found = candidatePaths.find((p) => fs.existsSync(p));
    this.binaryPath = found ?? path.resolve(currentDir, "../../../bin/forge_compute");
  }

  /**
   * Exécute un job de calcul scientifique sur le moteur natif C++.
   * Valide les types en entrée et en sortie via Zod.
   */
  async executeJob(rawRequest: ComputeJobRequest): Promise<ComputeJobResponse> {
    // 1. Validation Zod stricte en entrée
    const request = ComputeJobRequestSchema.parse(rawRequest);

    const args = [
      "--job-id",
      request.jobId,
      "--algorithm",
      request.algorithm,
      "--dimensions",
      request.dimensions.toString(),
      "--iterations",
      request.iterations.toString(),
    ];

    try {
      const { stdout } = await execFileAsync(this.binaryPath, args, {
        timeout: 10_000, // Timeout de sécurité 10s
        maxBuffer: 10 * 1024 * 1024,
      });

      // 2. Parser la réponse JSON émise par le binaire C++
      const rawJson = JSON.parse(stdout.trim());

      // 3. Validation Zod stricte de la sortie
      return ComputeJobResponseSchema.parse(rawJson);
    } catch (err: any) {
      if (err.code === "ENOENT") {
        throw new Error(
          `NATIVE_ENGINE_UNAVAILABLE: Le binaire natif est introuvable à l'emplacement ${this.binaryPath}. Avez-vous compilé native/ ?`
        );
      }
      throw new Error(`NATIVE_EXECUTION_ERROR: ${err.message}`);
    }
  }

  /**
   * Vérifie la disponibilité du binaire natif
   */
  async isHealthy(): Promise<boolean> {
    try {
      const res = await this.executeJob({
        jobId: "00000000-0000-0000-0000-000000000001",
        algorithm: "monte_carlo_pi",
        dimensions: 10,
        iterations: 100,
      });
      return res.status === "completed";
    } catch {
      return false;
    }
  }
}
