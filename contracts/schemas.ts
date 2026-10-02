import { z } from "zod";

/**
 * @forge/contracts — Schémas d'échanges universels
 * Règle d'or : Toute donnée franchissant une frontière (réseau, IPC, bridge) DOIT être validée ici.
 */

// --- 1. Diagnostic & Santé Système ---
export const HealthCheckResponseSchema = z.object({
  status: z.enum(["ok", "degraded", "error"]),
  version: z.string(),
  uptimeSeconds: z.number().nonnegative(),
  nativeEngineActive: z.boolean(),
  timestamp: z.string().datetime(),
});

export type HealthCheckResponse = z.infer<typeof HealthCheckResponseSchema>;

// --- 2. Calcul Scientifique & Moteur Numérique ---
export const ComputeJobRequestSchema = z.object({
  jobId: z.string().uuid(),
  algorithm: z.enum(["vector_dot_product", "matrix_multiply", "monte_carlo_pi", "fast_fourier_transform"]),
  dimensions: z.number().int().positive().max(10_000_000),
  iterations: z.number().int().positive().default(1),
  data: z.array(z.number()).optional(),
});

export const ComputeJobResponseSchema = z.object({
  jobId: z.string().uuid(),
  status: z.enum(["completed", "failed"]),
  executionTimeMs: z.number().nonnegative(),
  peakMemoryBytes: z.number().int().nonnegative(),
  resultChecksum: z.string(),
  summaryValue: z.number(),
  error: z.string().optional(),
});

export type ComputeJobRequest = z.infer<typeof ComputeJobRequestSchema>;
export type ComputeJobResponse = z.infer<typeof ComputeJobResponseSchema>;

// --- 3. Mesures de Performance (Mercuria) ---
export const BenchmarkMetricSchema = z.object({
  benchmarkName: z.string().min(1),
  opsPerSecond: z.number().positive(),
  meanLatencyNanos: z.number().positive(),
  p99LatencyNanos: z.number().positive(),
  allocationsCount: z.number().int().nonnegative(),
  bytesAllocated: z.number().int().nonnegative(),
});

export const BenchmarkReportSchema = z.object({
  timestamp: z.string().datetime(),
  environment: z.string(),
  metrics: z.array(BenchmarkMetricSchema),
  regressionDetected: z.boolean(),
});

export type BenchmarkMetric = z.infer<typeof BenchmarkMetricSchema>;
export type BenchmarkReport = z.infer<typeof BenchmarkReportSchema>;
