import { Router, Request, Response } from "express";
import { NativeBridge } from "../bridge/native_bridge.js";
import {
  ComputeJobRequestSchema,
  HealthCheckResponseSchema,
} from "@forge/contracts";

export function createComputeRouter(bridge: NativeBridge): Router {
  const router = Router();
  const startTime = Date.now();

  // Route de santé système
  router.get("/health", async (_req: Request, res: Response) => {
    const isEngineActive = await bridge.isHealthy();
    const uptime = (Date.now() - startTime) / 1000;

    const payload = HealthCheckResponseSchema.parse({
      status: isEngineActive ? "ok" : "degraded",
      version: "0.1.0",
      uptimeSeconds: uptime,
      nativeEngineActive: isEngineActive,
      timestamp: new Date().toISOString(),
    });

    res.json(payload);
  });

  // Route de calcul scientifique
  router.post("/compute", async (req: Request, res: Response) => {
    const parseResult = ComputeJobRequestSchema.safeParse(req.body);

    if (!parseResult.success) {
      res.status(400).json({
        success: false,
        error: {
          code: "VALIDATION_FAILED",
          message: "Le format de la requête ne respecte pas le contrat Zod.",
          details: parseResult.error.format(),
          timestamp: new Date().toISOString(),
        },
      });
      return;
    }

    try {
      const result = await bridge.executeJob(parseResult.data);
      res.json(result);
    } catch (err: any) {
      res.status(500).json({
        success: false,
        error: {
          code: "NATIVE_ENGINE_ERROR",
          message: err.message,
          timestamp: new Date().toISOString(),
        },
      });
    }
  });

  return router;
}
