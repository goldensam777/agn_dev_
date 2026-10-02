import React from "react";

interface MetricCardProps {
  label: string;
  value: string;
  color?: string;
}

export function MetricCard({ label, value, color = "#0f172a" }: MetricCardProps) {
  return (
    <div
      style={{
        background: "#ffffff",
        padding: 12,
        borderRadius: 6,
        border: "1px solid #dcfce7",
      }}
    >
      <div style={{ fontSize: 12, color: "#64748b" }}>{label}</div>
      <div style={{ fontSize: 18, fontWeight: 700, color }}>{value}</div>
    </div>
  );
}
