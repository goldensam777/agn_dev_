import express from "express";
import { NativeBridge } from "./bridge/native_bridge.js";
import { createComputeRouter } from "./routes/compute.js";

export function createApp(bridge?: NativeBridge) {
  const app = express();
  app.use(express.json());

  // CORS basique pour le développement
  app.use((_req, res, next) => {
    res.header("Access-Control-Allow-Origin", "*");
    res.header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    res.header("Access-Control-Allow-Headers", "Content-Type");
    next();
  });

  const nativeBridge = bridge ?? new NativeBridge();
  app.use("/api", createComputeRouter(nativeBridge));

  return app;
}
