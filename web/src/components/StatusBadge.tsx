import React from "react";
import { HealthCheckResponse } from "@forge/contracts";

interface StatusBadgeProps {
  health: HealthCheckResponse | null;
}

export function StatusBadge({ health }: StatusBadgeProps) {
  return (
    <section
      style={{
        background: "#f8fafc",
        padding: 16,
        borderRadius: 8,
        border: "1px solid #e2e8f0",
        marginBottom: 24,
        display: "flex",
        justifyContent: "space-between",
        alignItems: "center",
      }}
    >
      <div>
        <span style={{ fontWeight: 600, color: "#334155" }}>Statut Moteur Natif : </span>
        <span
          style={{
            color: health?.nativeEngineActive ? "#16a34a" : "#dc2626",
            fontWeight: 700,
          }}
        >
          {health?.nativeEngineActive ? "● EN LIGNE (C++ / SIMD Actif)" : "○ HORS LIGNE"}
        </span>
      </div>
      {health && (
        <div style={{ color: "#64748b", fontSize: 13 }}>
          Version {health.version} | Uptime: {Math.round(health.uptimeSeconds)}s
        </div>
      )}
    </section>
  );
}
