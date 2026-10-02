/**
 * @file canonical_scientific_canvas.tsx
 * @brief Modèle canonique React 19 : visualisation scientifique WebGL sans fuite.
 *
 * Règles illustrées (corpus .harness/knowledge/languages/react/) :
 *  - domains/scientific-viz.md  : canvas isolé, rendu IMPÉRATIF via ref — les
 *    ~60 fps ne passent JAMAIS par le reconciler.
 *  - core/lifecycle-cleanup.md  : TOUT ce qui est créé dans l'effet est détruit
 *    dans le cleanup — sûr sous StrictMode (double mount) et en navigation SPA.
 *  - core/rendering-hooks.md    : le composant reste PUR : mêmes props → même rendu.
 *
 * NB : les shaders sont volontairement minimaux — l'exemple porte sur le
 * CYCLE DE VIE du contexte GPU, pas sur GLSL.
 */

import { useEffect, useRef } from "react";

export interface SpectrogramProps {
  /** Buffer contigu reçu PAR RÉFÉRENCE — jamais recopié dans un état React. */
  readonly magnitudes: Float32Array;
  readonly colormap: "viridis" | "inferno";
}

const VS_SOURCE = `#version 300 es
layout(location = 0) in vec2 a_pos;
out vec2 v_uv;
void main() { v_uv = a_pos * 0.5 + 0.5; gl_Position = vec4(a_pos, 0.0, 1.0); }`;

const FRAGMENT_SOURCES: Record<SpectrogramProps["colormap"], string> = {
  viridis: `#version 300 es
precision mediump float; in vec2 v_uv; out vec4 o; void main() { o = vec4(v_uv, 0.4, 1.0); }`,
  inferno: `#version 300 es
precision mediump float; in vec2 v_uv; out vec4 o; void main() { o = vec4(1.0, v_uv.x, 0.1, 1.0); }`,
};

export function SpectrogramCanvas({ magnitudes, colormap }: SpectrogramProps) {
  const canvasRef = useRef<HTMLCanvasElement>(null);

  useEffect(() => {
    const canvas = canvasRef.current;
    if (canvas === null) return; // jamais de non-null assertion
    const gl = canvas.getContext("webgl2", { antialias: false });
    if (gl === null) return;

    const program = buildProgram(gl, FRAGMENT_SOURCES[colormap]);
    if (program === null) return;

    // VBO dynamique : les données sont POUSSÉES à chaque frame, jamais stockées en état.
    const vbo = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vbo);
    gl.bufferData(gl.ARRAY_BUFFER, magnitudes.byteLength, gl.DYNAMIC_DRAW);

    let raf = 0;
    const render = () => {
      gl.bufferSubData(gl.ARRAY_BUFFER, 0, magnitudes);
      gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
      raf = requestAnimationFrame(render);
    };

    // Le canvas suit son conteneur ; l'observer est débranché au cleanup.
    const observer = new ResizeObserver((entries) => {
      for (const entry of entries) {
        canvas.width = Math.max(1, Math.floor(entry.contentRect.width));
        canvas.height = Math.max(1, Math.floor(entry.contentRect.height));
        gl.viewport(0, 0, canvas.width, canvas.height);
      }
    });
    observer.observe(canvas);
    raf = requestAnimationFrame(render);

    // CLEANUP INTÉGRAL — la moitié de l'effet est cette fonction de retour.
    return () => {
      cancelAnimationFrame(raf);
      observer.disconnect();
      gl.deleteBuffer(vbo);
      gl.deleteProgram(program);
      // SANS cette ligne : des centaines de mounts épuisent les contextes GPU du navigateur.
      gl.getExtension("WEBGL_lose_context")?.loseContext();
    };
  }, [magnitudes, colormap]); // seules raisons légitimes de re-renderer

  // Rendu déclaratif minimal : React ne touche jamais au contenu du canvas.
  return (
    <canvas
      ref={canvasRef}
      style={{ width: "100%", height: "100%", display: "block" }}
      aria-label="Spectrogramme scientifique"
    />
  );
}

function compileShader(
  gl: WebGL2RenderingContext,
  type: number,
  source: string,
): WebGLShader | null {
  const shader = gl.createShader(type);
  if (shader === null) return null;
  gl.shaderSource(shader, source);
  gl.compileShader(shader);
  if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
    // Les erreurs shader DOIVENT devenir des diagnostics, pas des écrans noirs.
    console.error("Compilation shader échouée :", gl.getShaderInfoLog(shader));
    gl.deleteShader(shader);
    return null;
  }
  return shader;
}

function buildProgram(
  gl: WebGL2RenderingContext,
  fragmentSource: string,
): WebGLProgram | null {
  const vs = compileShader(gl, gl.VERTEX_SHADER, VS_SOURCE);
  const fs = compileShader(gl, gl.FRAGMENT_SHADER, fragmentSource);
  if (vs === null || fs === null) return null;
  const program = gl.createProgram();
  if (program === null) return null;
  gl.attachShader(program, vs);
  gl.attachShader(program, fs);
  gl.linkProgram(program);
  gl.deleteShader(vs);
  gl.deleteShader(fs);
  if (!gl.getProgramParameter(program, gl.LINK_STATUS)) {
    console.error("Lien shader échoué :", gl.getProgramInfoLog(program));
    gl.deleteProgram(program);
    return null;
  }
  return program;
}
