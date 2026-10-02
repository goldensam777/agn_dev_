import { createApp } from "./app.js";

const PORT = process.env.PORT ? parseInt(process.env.PORT, 10) : 3001;

const app = createApp();

app.listen(PORT, () => {
  console.log(`[FORGE SERVER] Serveur démarré avec succès sur http://localhost:${PORT}`);
  console.log(`[FORGE SERVER] Endpoint santé : http://localhost:${PORT}/api/health`);
  console.log(`[FORGE SERVER] Endpoint calcul : http://localhost:${PORT}/api/compute`);
});
