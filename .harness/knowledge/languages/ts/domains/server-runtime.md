# Domaine TS : Runtime Serveur Node.js & Boucle d'Événements

> Ce document établit les règles d'exécution serveur haute fiabilité sous Node.js / TypeScript.

---

## 1. Respect Strict de la Boucle d'Événements (Event Loop)

- Ne JAMAIS bloquer le thread principal avec des opérations de hachage intensif, du chiffrement ou du parsing synchrone de JSON volumineux (> 10 Mo).
- Les calculs CPU lourds doivent impérativement être délégués :
  - Soit au moteur natif C++ via le bridge.
  - Soit à des `worker_threads` Node.js.

---

## 2. Fermeture Propre (*Graceful Shutdown*)

Tout serveur Node.js doit intercepter les signaux d'arrêt système pour libérer ses ressources :
```typescript
function setupGracefulShutdown(server: Server) {
  const shutdown = (signal: string) => {
    console.log(`Signal ${signal} reçu. Fermeture ordonnée du serveur...`);
    server.close(() => {
      console.log("Connexions HTTP terminées.");
      process.exit(0);
    });
    // Force l'arrêt après 5 secondes si des sockets restent ouvertes
    setTimeout(() => process.exit(1), 5000).unref();
  };

  process.on("SIGTERM", () => shutdown("SIGTERM"));
  process.on("SIGINT", () => shutdown("SIGINT"));
}
```

---

## 3. Checklist Actionnable pour l'Agent

- [ ] Aucune opération synchrone de calcul lourd n'est exécutée sur le thread principal ?
- [ ] Les signaux `SIGTERM` et `SIGINT` sont-ils capturés pour une fermeture propre (*graceful shutdown*) ?
- [ ] Les promesses non capturées (`unhandledRejection`) sont-elles surveillées ?
