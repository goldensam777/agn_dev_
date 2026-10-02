import React, { useState, useEffect } from "react";
import { fetchHealth, submitComputeJob } from "../api/client.js";
import { HealthCheckResponse, ComputeJobResponse } from "@forge/contracts";
import { StatusBadge } from "../components/StatusBadge.js";
import { ControlPanel, AlgorithmType } from "../components/ControlPanel.js";
import { MetricCard } from "../components/MetricCard.js";

export function DashboardPage() {
  const [health, setHealth] = useState<HealthCheckResponse | null>(null);
  const [algorithm, setAlgorithm] = useState<AlgorithmType>("monte_carlo_pi");
  const [iterations, setIterations] = useState<number>(500_000);
  const [dimensions, setDimensions] = useState<number>(100_000);
  const [loading, setLoading] = useState<boolean>(false);
  const [result, setResult] = useState<ComputeJobResponse | null>(null);
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    fetchHealth()
      .then(setHealth)
      .catch((err) => setError(`Serveur indisponible : ${err.message}`));
  }, []);

  const handleRunJob = async () => {
    setLoading(true);
    setError(null);
    try {
      const res = await submitComputeJob({
        jobId: crypto.randomUUID(),
        algorithm,
        iterations,
        dimensions,
      });
      setResult(res);
    } catch (err: any) {
      setError(err.message);
    } finally {
      setLoading(false);
    }
  };

  return (
    <div
      style={{
        fontFamily: "system-ui, -apple-system, sans-serif",
        maxWidth: 900,
        margin: "40px auto",
        padding: "0 20px",
      }}
    >
      <header
        style={{
          borderBottom: "2px solid #2563eb",
          paddingBottom: 16,
          marginBottom: 32,
        }}
      >
        <h1 style={{ margin: 0, color: "#1e293b", fontSize: 28 }}>
          ⚡ Forge Agentique — Tableau de Bord Scientifique
        </h1>
        <p style={{ margin: "8px 0 0", color: "#64748b" }}>
          Calcul numérique C++ orchestré par Node.js / TypeScript & Validé par Zod
        </p>
      </header>

      <StatusBadge health={health} />

      <ControlPanel
        algorithm={algorithm}
        setAlgorithm={setAlgorithm}
        iterations={iterations}
        setIterations={setIterations}
        dimensions={dimensions}
        setDimensions={setDimensions}
        loading={loading}
        onRun={handleRunJob}
      />

      {error && (
        <div
          style={{
            background: "#fef2f2",
            border: "1px solid #fecaca",
            color: "#b91c1c",
            padding: 16,
            borderRadius: 6,
            marginBottom: 24,
          }}
        >
          <strong>Erreur : </strong> {error}
        </div>
      )}

      {result && (
        <section
          style={{
            background: "#f0fdf4",
            border: "1px solid #bbf7d0",
            borderRadius: 8,
            padding: 24,
          }}
        >
          <h2 style={{ fontSize: 18, margin: "0 0 16px", color: "#166534" }}>
            ✓ Résultat du Calcul Numérique
          </h2>
          <div
            style={{
              display: "grid",
              gridTemplateColumns: "repeat(4, 1fr)",
              gap: 16,
            }}
          >
            <MetricCard
              label="Valeur Calculée"
              value={result.summaryValue.toFixed(6)}
            />
            <MetricCard
              label="Temps d'exécution"
              value={`${result.executionTimeMs.toFixed(2)} ms`}
              color="#2563eb"
            />
            <MetricCard
              label="Mémoire Pic (RSS)"
              value={`${(result.peakMemoryBytes / (1024 * 1024)).toFixed(1)} Mo`}
            />
            <MetricCard
              label="Checksum Sécurité"
              value={result.resultChecksum}
              color="#475569"
            />
          </div>
        </section>
      )}
    </div>
  );
}
