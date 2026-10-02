import {
  ComputeJobRequest,
  ComputeJobRequestSchema,
  ComputeJobResponse,
  ComputeJobResponseSchema,
  HealthCheckResponse,
  HealthCheckResponseSchema,
} from "@forge/contracts";

const API_BASE = "http://localhost:3001/api";

export async function fetchHealth(): Promise<HealthCheckResponse> {
  const res = await fetch(`${API_BASE}/health`);
  if (!res.ok) {
    throw new Error(`HTTP Error ${res.status}`);
  }
  const data = await res.json();
  return HealthCheckResponseSchema.parse(data);
}

export async function submitComputeJob(
  job: ComputeJobRequest
): Promise<ComputeJobResponse> {
  const validatedPayload = ComputeJobRequestSchema.parse(job);

  const res = await fetch(`${API_BASE}/compute`, {
    method: "POST",
    headers: {
      "Content-Type": "application/json",
    },
    body: JSON.stringify(validatedPayload),
  });

  if (!res.ok) {
    const errorJson = await res.json().catch(() => null);
    throw new Error(
      errorJson?.error?.message ?? `Erreur serveur HTTP ${res.status}`
    );
  }

  const data = await res.json();
  return ComputeJobResponseSchema.parse(data);
}
