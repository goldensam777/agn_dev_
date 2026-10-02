import React from "react";

export type AlgorithmType =
  | "monte_carlo_pi"
  | "vector_dot_product"
  | "matrix_multiply";

interface ControlPanelProps {
  algorithm: AlgorithmType;
  setAlgorithm: (algo: AlgorithmType) => void;
  iterations: number;
  setIterations: (n: number) => void;
  dimensions: number;
  setDimensions: (d: number) => void;
  loading: boolean;
  onRun: () => void;
}

export function ControlPanel({
  algorithm,
  setAlgorithm,
  iterations,
  setIterations,
  dimensions,
  setDimensions,
  loading,
  onRun,
}: ControlPanelProps) {
  return (
    <section
      style={{
        background: "#ffffff",
        padding: 24,
        borderRadius: 8,
        border: "1px solid #cbd5e1",
        marginBottom: 24,
      }}
    >
      <h2 style={{ fontSize: 18, margin: "0 0 16px", color: "#0f172a" }}>
        Paramètres d'Exécution
      </h2>

      <div
        style={{
          display: "grid",
          gridTemplateColumns: "1fr 1fr",
          gap: 16,
          marginBottom: 20,
        }}
      >
        <div>
          <label
            style={{
              display: "block",
              fontSize: 14,
              fontWeight: 500,
              marginBottom: 6,
              color: "#475569",
            }}
          >
            Algorithme de calcul
          </label>
          <select
            value={algorithm}
            onChange={(e) => setAlgorithm(e.target.value as AlgorithmType)}
            style={{
              width: "100%",
              padding: "8px 12px",
              borderRadius: 6,
              border: "1px solid #cbd5e1",
            }}
          >
            <option value="monte_carlo_pi">Monte Carlo Pi (Stochastique)</option>
            <option value="vector_dot_product">
              Produit Scalaire (SIMD-Friendly)
            </option>
            <option value="matrix_multiply">
              Multiplication Matricielle (O(N^3))
            </option>
          </select>
        </div>

        <div>
          <label
            style={{
              display: "block",
              fontSize: 14,
              fontWeight: 500,
              marginBottom: 6,
              color: "#475569",
            }}
          >
            {algorithm === "monte_carlo_pi"
              ? "Nombre d'itérations"
              : "Dimension des vecteurs/matrices"}
          </label>
          <input
            type="number"
            value={algorithm === "monte_carlo_pi" ? iterations : dimensions}
            onChange={(e) => {
              const val = parseInt(e.target.value, 10);
              if (algorithm === "monte_carlo_pi") setIterations(val);
              else setDimensions(val);
            }}
            style={{
              width: "95%",
              padding: "8px 12px",
              borderRadius: 6,
              border: "1px solid #cbd5e1",
            }}
          />
        </div>
      </div>

      <button
        onClick={onRun}
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
        {loading
          ? "Calcul en cours dans le moteur C++..."
          : "Lancer le Calcul Haute Performance"}
      </button>
    </section>
  );
}
