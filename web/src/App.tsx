import React, { useState, useEffect } from "react";
import { fetchHealth, submitComputeJob } from "./api/client.js";
import {
  HealthCheckResponse,
  ComputeJobResponse,
} from "@forge/contracts";

export function App() {
  const [health, setHealth] = useState<HealthCheckResponse | null>(null);
  const [algorithm, setAlgorithm] = useState<
    "monte_carlo_pi" | "vector_dot_product" | "matrix_multiply"
  >("monte_carlo_pi");
  const [iterations, setIterations] = useState<number>(500_000);
  const [dimensions, setDimensions] = useState<number>(100_000);
  const [loading, setLoading] = useState<boolean>(false);
  const [result, setResult] = useState<ComputeJobResponse | null>(null);
  const [error, setError] = useState<string | null>(null);

  // Vérifier la santé du backend au chargement
  useEffect(() => {
    fetchHealth()
      .then(setHealth)
      .catch((err) =>
        setError(`Serveur indisponible : ${err.message}`)
      );
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
    <div style={{ fontFamily: "system-ui, -apple-system, sans-serif", maxWidth: 900, margin: "40px auto", padding: "0 20px" }}>
      <header style={{ borderBottom: "2px solid #2563eb", paddingBottom: 16, marginBottom: 32 }}>
        <h1 style={{ margin: 0, color: "#1e293b", fontSize: 28 }}>
          ⚡ Forge Agentique — Tableau de Bord Scientifique
        </h1>
        <p style={{ margin: "8px 0 0", color: "#64748b" }}>
          Calcul numérique C++ orchestré par Node.js / TypeScript & Validé par Zod
        </p>
      </header>

      {/* Barre de Statut Système */}
      <section style={{ background: "#f8fafc", padding: 16, borderRadius: 8, border: "1px solid #e2e8f0", marginBottom: 24, display: "flex", justifyContent: "space-between", alignItems: "center" }}>
        <div>
          <span style={{ fontWeight: 600, color: "#334155" }}>Statut Moteur Natif : </span>
          <span style={{ color: health?.nativeEngineActive ? "#16a34a" : "#dc2626", fontWeight: 700 }}>
            {health?.nativeEngineActive ? "● EN LIGNE (C++ / SIMD Actif)" : "○ HORS LIGNE"}
          </span>
        </div>
        {health && (
          <div style={{ color: "#64748b", fontSize: 13 }}>
            Version {health.version} | Uptime: {Math.round(health.uptimeSeconds)}s
          </div>
        )}
      </section>

      {/* Panneau de Contrôle du Calcul */}
      <section style={{ background: "#ffffff", padding: 24, borderRadius: 8, border: "1px solid #cbd5e1", marginBottom: 24 }}>
        <h2 style={{ fontSize: 18, margin: "0 0 16px", color: "#0f172a" }}>Paramètres d'Exécution</h2>
        
        <div style={{ display: "grid", gridTemplateColumns: "1fr 1fr", gap: 16, marginBottom: 20 }}>
          <div>
            <label style={{ display: "block", fontSize: 14, fontWeight: 500, marginBottom: 6, color: "#475569" }}>
              Algorithme de calcul
            </label>
            <select
              value={algorithm}
              onChange={(e) => setAlgorithm(e.target.value as any)}
              style={{ width: "100%", padding: "8px 12px", borderRadius: 6, border: "1px solid #cbd5e1" }}
            >
              <option value="monte_carlo_pi">Monte Carlo Pi (Stochastique)</option>
              <option value="vector_dot_product">Produit Scalaire (SIMD-Friendly)</option>
              <option value="matrix_multiply">Multiplication Matricielle (O(N^3))</option>
            </select>
          </div>

          <div>
            <label style={{ display: "block", fontSize: 14, fontWeight: 500, marginBottom: 6, color: "#475569" }}>
              {algorithm === "monte_carlo_pi" ? "Nombre d'itérations" : "Dimension des vecteurs/matrices"}
            </label>
            <input
              type="number"
              value={algorithm === "monte_carlo_pi" ? iterations : dimensions}
              onChange={(e) => {
                const val = parseInt(e.target.value, 10);
                if (algorithm === "monte_carlo_pi") setIterations(val);
                else setDimensions(val);
              }}
              style={{ width: "95%", padding: "8px 12px", borderRadius: 6, border: "1px solid #cbd5e1" }}
            />
          </div>
        </div>

        <button
          onClick={handleRunJob}
          disabled={loading}
          style={{
            background: loading ? "#94a3b8" : "#2563eb",
            color: "#ffffff",
            border: "none",
            borderRadius: 6,
            padding: "10px 24px",
            fontSize: 15,
            fontWeight: 600,
            cursor: loading ? "not-allowed" : "pointer",
          }}
        >
          {loading ? "Calcul en cours dans le moteur C++..." : "Lancer le Calcul Haute Performance"}
        </button>
      </section>

      {/* Affichage des Erreurs */}
      {error && (
        <div style={{ background: "#fef2f2", border: "1px solid #fecaca", color: "#b91c1c", padding: 16, borderRadius: 6, marginBottom: 24 }}>
          <strong>Erreur : </strong> {error}
        </div>
      )}

      {/* Affichage des Résultats Typés */}
      {result && (
        <section style={{ background: "#f0fdf4", border: "1px solid #bbf7d0", borderRadius: 8, padding: 24 }}>
          <h2 style={{ fontSize: 18, margin: "0 0 16px", color: "#166534" }}>✓ Résultat du Calcul Numérique</h2>
          <div style={{ display: "grid", gridTemplateColumns: "repeat(4, 1fr)", gap: 16, marginBottom: 16 }}>
            <div style={{ background: "#ffffff", padding: 12, borderRadius: 6, border: "1px solid #dcfce7" }}>
              <div style={{ fontSize: 12, color: "#64748b" }}>Valeur Calculée</div>
              <div style={{ fontSize: 20, fontWeight: 700, color: "#0f172a" }}>
                {result.summaryValue.toFixed(6)}
              </div>
            </div>
            <div style={{ background: "#ffffff", padding: 12, borderRadius: 6, border: "1px solid #dcfce7" }}>
              <div style={{ fontSize: 12, color: "#64748b" }}>Temps d'exécution</div>
              <div style={{ fontSize: 20, fontWeight: 700, color: "#2563eb" }}>
                {result.executionTimeMs.toFixed(2)} ms
              </div>
            </div>
            <div style={{ background: "#ffffff", padding: 12, borderRadius: 6, border: "1px solid #dcfce7" }}>
              <div style={{ fontSize: 12, color: "#64748b" }}>Mémoire Pic (RSS)</div>
              <div style={{ fontSize: 20, fontWeight: 700, color: "#0f172a" }}>
                {(result.peakMemoryBytes / (1024 * 1024)).toFixed(1)} Mo
              </div>
            </div>
            <div style={{ background: "#ffffff", padding: 12, borderRadius: 6, border: "1px solid #dcfce7" }}>
              <div style={{ fontSize: 12, color: "#64748b" }}>Checksum Sécurité</div>
              <div style={{ fontSize: 16, fontWeight: 600, color: "#475569" }}>
                {result.resultChecksum}
              </div>
            </div>
          </div>
        </section>
      )}
    </div>
  );
}
