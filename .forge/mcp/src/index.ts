import { Server } from "@modelcontextprotocol/sdk/server/index.js";
import { StdioServerTransport } from "@modelcontextprotocol/sdk/server/stdio.js";
import {
  CallToolRequestSchema,
  ListToolsRequestSchema,
} from "@modelcontextprotocol/sdk/types.js";
import { exec } from "node:child_process";
import { promisify } from "node:util";
import fs from "node:fs/promises";
import path from "node:path";

const execAsync = promisify(exec);

const EXEC_OPTIONS = {
  timeout: 60_000, // 60 secondes maximum
  maxBuffer: 10 * 1024 * 1024, // 10 Mo de buffer pour les traces complètes
};

/**
 * Serveur MCP Forge — Surcouche d'outils de haute précision pour agents de laboratoire
 */
const server = new Server(
  {
    name: "forge-agent-tools",
    version: "0.2.0",
  },
  {
    capabilities: {
      tools: {},
    },
  }
);

// Outils chirurgicaux exposés à l'agent
const TOOLS = [
  {
    name: "forge_verify",
    description: "Exécute le juge scripts/verify.sh. Vérifie la compilation, l'absence de fuites mémoire (ASan), les types Zod et les tests. Le code de retour 0 est la seule preuve de succès.",
    inputSchema: {
      type: "object",
      properties: {
        verbose: { type: "boolean", description: "Afficher tous les détails de sortie" },
      },
    },
  },
  {
    name: "forge_audit_memory",
    description: "Compile et exécute un fichier source C++ isolé sous AddressSanitizer (ASan). Vérifie l'absence de fuite et de dépassement de tampon dans un binaire temporaire sécurisé avec timeout.",
    inputSchema: {
      type: "object",
      properties: {
        sourcePath: { type: "string", description: "Chemin du fichier source C++ à analyser" },
      },
      required: ["sourcePath"],
    },
  },
  {
    name: "forge_run_benchmarks",
    description: "Compile et exécute réellement le banc de mesure natif (make bench). Retourne les métriques réelles de débit (opérations/seconde) et de latence.",
    inputSchema: {
      type: "object",
      properties: {},
    },
  },
];

server.setRequestHandler(ListToolsRequestSchema, async () => {
  return { tools: TOOLS };
});

server.setRequestHandler(CallToolRequestSchema, async (request) => {
  const { name, arguments: args } = request.params;

  try {
    if (name === "forge_verify") {
      try {
        const { stdout, stderr } = await execAsync("bash scripts/verify.sh", {
          ...EXEC_OPTIONS,
          timeout: 120_000,
        });
        return {
          content: [
            {
              type: "text",
              text: `=== VERDICT FORGE: PASS ===\n${stdout}\n${stderr}`,
            },
          ],
        };
      } catch (err: any) {
        return {
          isError: true,
          content: [
            {
              type: "text",
              text: `=== VERDICT FORGE: FAIL (Code de retour ${err.code ?? 1}) ===\n${err.stdout ?? ""}\n${err.stderr ?? ""}\nErreur: ${err.message}`,
            },
          ],
        };
      }
    }

    if (name === "forge_audit_memory") {
      const source = String(args?.["sourcePath"] ?? "");
      const resolvedSource = path.resolve(process.cwd(), source);

      // Génération d'un nom de binaire temporaire unique pour éviter les collisions
      const uniqueSuffix = `${Date.now()}_${Math.random().toString(36).substring(2, 8)}`;
      const tempBin = `/tmp/forge_asan_${uniqueSuffix}`;

      const compileAndRunCmd = `clang++ -std=c++20 -Wall -Wextra -Werror -fsanitize=address,undefined -I native/include "${resolvedSource}" -o "${tempBin}" && "${tempBin}"`;

      try {
        const { stdout, stderr } = await execAsync(compileAndRunCmd, EXEC_OPTIONS);
        return {
          content: [
            {
              type: "text",
              text: `=== AUDIT MÉMOIRE RÉEL : 0 ERREUR DÉTECTÉE ===\nSortie standard:\n${stdout}\nDiagnostic ASan:\n${stderr}`,
            },
          ],
        };
      } finally {
        // Nettoyage garanti du binaire temporaire
        await fs.unlink(tempBin).catch(() => {});
      }
    }

    if (name === "forge_run_benchmarks") {
      // Exécution RÉELLE du banc de test de performance natif
      const { stdout, stderr } = await execAsync("make -f native/Makefile bench", EXEC_OPTIONS);
      return {
        content: [
          {
            type: "text",
            text: `=== MESURES RÉELLES DU BANC DE PERFORMANCE ===\n${stdout}\n${stderr}`,
          },
        ],
      };
    }

    throw new Error(`Outil inconnu : ${name}`);
  } catch (error: any) {
    return {
      isError: true,
      content: [
        {
          type: "text",
          text: `ÉCHEC DE L'OPÉRATION FORGE:\n${error.message}\nSortie d'erreur:\n${error.stderr || ""}`,
        },
      ],
    };
  }
});

async function main() {
  const transport = new StdioServerTransport();
  await server.connect(transport);
  console.error("Serveur MCP Forge v0.2.0 démarré sur le transport standard (stdio).");
}

main().catch((err) => {
  console.error("Erreur critique serveur MCP:", err);
  process.exit(1);
});
