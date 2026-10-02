# Domaine React : Visualisation Scientifique Haute Fréquence (WebGL / WebGPU)

> Ce document établit le protocole d'intégration de rendus graphiques 2D/3D et simulations temps réel à 60 fps sans pénaliser React.

---

## 1. La Règle d'Or : Isoler la Boucle de Rendu du Cycle de React

Dans une visualisation scientifique (tracé de 100 000 points, simulation d'ondes, rendu WebGPU) :
- **Ne JAMAIS faire re-rendre le composant React à 60 images par seconde.**
- React doit uniquement monter le `<canvas ref={canvasRef} />`.
- Toute la boucle de dessin doit s'exécuter dans un contexte graphique impératif via `requestAnimationFrame` imperméable aux états React.

```tsx
import { useEffect, useRef } from "react";

export function SimulationCanvas({ simulationData }: { simulationData: Float32Array }) {
  const canvasRef = useRef<HTMLCanvasElement | null>(null);

  useEffect(() => {
    const canvas = canvasRef.current;
    if (!canvas) return;
    const gl = canvas.getContext("webgl2");
    if (!gl) return;

    let animationFrameId: number;

    const renderLoop = (time: number) => {
      // 1. Mise à jour des buffers GPU et dessin WebGL
      drawScientificFrame(gl, simulationData);

      // 2. Boucle continue
      animationFrameId = requestAnimationFrame(renderLoop);
    };

    animationFrameId = requestAnimationFrame(renderLoop);

    // Nettoyage impératif
    return () => {
      cancelAnimationFrame(animationFrameId);
    };
  }, [simulationData]);

  return <canvas ref={canvasRef} width={800} height={600} />;
}
```

---

## 2. Transfert Zero-Copy avec les TypedArrays

- Pour transférer des données massives entre le serveur/bridge et le canvas :
  - Toujours utiliser `Float64Array` ou `Float32Array` directement vers les shaders WebGL/WebGPU.
  - Ne jamais mapper un buffer binaire en tableau d'objets JavaScript (`[{x, y}, ...]`), qui consomme 4 fois plus de mémoire et sature le garbage collector.

---

## 3. Checklist Actionnable pour l'Agent

- [ ] La boucle de dessin tourne-t-elle sous `requestAnimationFrame` sans déclencher de `setState` ?
- [ ] Tout `requestAnimationFrame` est-il nettoyé avec `cancelAnimationFrame` lors du démontage ?
- [ ] Les données volumineuses sont-elles transmises au GPU sous forme de `TypedArray` ?
