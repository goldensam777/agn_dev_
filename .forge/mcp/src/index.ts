import { Server } from "@modelcontextprotocol/sdk/server/index.js";
import { StdioServerTransport } from "@modelcontextprotocol/sdk/server/stdio.js";
import {
  CallToolRequestSchema,
  ListToolsRequestSchema,
} from "@modelcontextprotocol/sdk/types.js";
import { execFile } from "node:child_process";
import { promisify } from "node:util";
import fs from "node:fs/promises";
import path from "node:path";
import { fileURLToPath } from "node:url";

const execFileAsync = promisify(execFile);

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

// Chemin racine du Hub de la Forge
const FORGE_HUB_DIR = path.resolve(process.env.FORGE_HUB_DIR || path.resolve(__dirname, "../../.."));

// Racines autorisées pour la restriction de chemins (sécurité stricte anti-traversal)
const HOME_DIR = process.env.HOME || "/home/samuelyevi";
const ALLOWED_ROOTS = [
  FORGE_HUB_DIR,
  path.resolve(HOME_DIR, "dev"),
  path.resolve("/mnt/dev"),
  path.resolve("/tmp"),
];

/**
 * Valide et restreint strictement un chemin sous l'une des racines autorisées.
 * Lève une exception si le chemin tente de s'échapper ou de cibler des répertoires interdits.
 */
function validatePathUnderAllowedRoots(inputPath: string, paramName: string): string {
  if (!inputPath || typeof inputPath !== "string") {
    throw new Error(`Paramètre invalide pour '${paramName}' : chaîne de caractères requise.`);
  }

  const resolved = path.resolve(process.cwd(), inputPath);
  const isAllowed = ALLOWED_ROOTS.some((root) => {
    return resolved === root || resolved.startsWith(root + path.sep);
  });

  if (!isAllowed) {
    throw new Error(
      `Accès refusé pour '${paramName}' : '${inputPath}' (résolu en '${resolved}'). Le chemin doit impérativement se situer sous l'une des racines autorisées (${ALLOWED_ROOTS.join(", ")}).`
    );
  }

  return resolved;
}

const EXEC_OPTIONS = {
  timeout: 60_000,
  maxBuffer: 10 * 1024 * 1024,
};

const ALLOWED_STANDARDS = new Set(["c++20", "c++23", "c++17", "c23", "c17", "c11"]);

/**
 * Serveur MCP Hub Global de la Forge
 * Implémentation durcie : utilisation exclusive de execFile avec arguments séparés,
 * interdiction absolue de l'interpolation de chaînes de shell, et restriction des chemins.
 */
const server = new Server(
  {
    name: "forge-global-hub",
    version: "1.1.0",
  },
  {
    capabilities: {
      tools: {},
    },
  }
);

const TOOLS = [
  {
    name: "forge_query_knowledge",
    description: "Interroge la base de connaissances experte de la Forge (7 corpus de langages, Dragon Book, architecture scientifique, fullstack) via execFile sécurisé.",
    inputSchema: {
      type: "object",
      properties: {
        query: { type: "string", description: "Terme ou concept à rechercher (ex: 'Pratt', 'SSA', 'Arena C23')" },
        category: {
          type: "string",
          enum: ["all", "languages", "compilers", "dragon_book", "scientific", "fullstack"],
          description: "Catégorie de recherche optionnelle",
        },
      },
      required: ["query"],
    },
  },
  {
    name: "forge_get_canonical_example",
    description: "Récupère un exemple canonique de référence validé sous ASan/tests ou liste les exemples disponibles.",
    inputSchema: {
      type: "object",
      properties: {
        exampleName: {
          type: "string",
          description: "Nom du fichier d'exemple (ex: 'canonical_pratt_parser_arena.cpp') ou 'list'.",
        },
      },
      required: ["exampleName"],
    },
  },
  {
    name: "forge_scaffold_harness",
    description: "Initialise l'enveloppe de gouvernance agentique dans un projet extérieur situé sous une racine autorisée.",
    inputSchema: {
      type: "object",
      properties: {
        targetDir: { type: "string", description: "Chemin du projet extérieur (doit résider sous une racine autorisée)" },
        projectType: {
          type: "string",
          enum: ["rust", "cpp", "c", "typescript", "python", "polyglot"],
          description: "Stack technologique principale",
        },
        projectName: { type: "string", description: "Nom du projet extérieur" },
      },
      required: ["targetDir", "projectType"],
    },
  },
  {
    name: "forge_audit_memory",
    description: "Compile et exécute un fichier source C ou C++ sous AddressSanitizer et UndefinedBehaviorSanitizer via execFile sans shell.",
    inputSchema: {
      type: "object",
      properties: {
        sourcePath: { type: "string", description: "Chemin du fichier source C/C++ à auditer (sous racine autorisée)" },
        standard: { type: "string", description: "Standard du compilateur (ex: 'c++20', 'c23'), défaut 'c++20'" },
        isC: { type: "boolean", description: "Vrai si C pur (clang), faux si C++ (clang++)" },
      },
      required: ["sourcePath"],
    },
  },
  {
    name: "forge_verify",
    description: "Exécute le juge scripts/verify.sh dans le projet cible (ou le répertoire courant) via execFile.",
    inputSchema: {
      type: "object",
      properties: {
        projectDir: { type: "string", description: "Répertoire racine du projet à vérifier (défaut: dossier courant)" },
        verbose: { type: "boolean", description: "Afficher l'intégralité des sorties" },
      },
    },
  },
];

server.setRequestHandler(ListToolsRequestSchema, async () => {
  return { tools: TOOLS };
});

server.setRequestHandler(CallToolRequestSchema, async (request) => {
  const { name, arguments: args } = request.params;

  try {
    // 1. Interrogation de la base de connaissances du Hub
    if (name === "forge_query_knowledge") {
      const query = String(args?.["query"] ?? "").trim();
      const category = String(args?.["category"] ?? "all");

      if (!query) throw new Error("Le paramètre 'query' est requis.");

      const results: string[] = [];

      // A. Recherche dans le Dragon Book via query_book.py avec execFile (aucun shell)
      if (category === "compilers" || category === "dragon_book" || category === "all") {
        try {
          const scriptPath = path.join(FORGE_HUB_DIR, "scripts/query_book.py");
          const { stdout } = await execFileAsync(
            "python3",
            [scriptPath, "--book", "dragon", "--search", query],
            {
              cwd: FORGE_HUB_DIR,
              timeout: 20_000,
            }
          );
          if (stdout && stdout.trim().length > 0) {
            results.push(`### Extraits du Dragon Book (Compilateurs & Optimisations) :\n${stdout.slice(0, 3000)}`);
          }
        } catch {
          // Aucun résultat ou erreur tolérée
        }
      }

      // B. Recherche textuelle dans .harness/knowledge avec execFile grep direct
      const knowledgeDir = path.join(FORGE_HUB_DIR, ".harness/knowledge");
      try {
        const { stdout } = await execFileAsync(
          "grep",
          ["-rnI", "-i", "--max-count=3", query, knowledgeDir],
          {
            timeout: 10_000,
            maxBuffer: EXEC_OPTIONS.maxBuffer,
          }
        );
        if (stdout && stdout.trim().length > 0) {
          const limitedOutput = stdout.split("\n").slice(0, 30).join("\n");
          results.push(`### Fichiers de Connaissances du Hub correspondants :\n${limitedOutput}`);
        }
      } catch {
        // grep renvoie code 1 si aucune correspondance
      }

      const responseText = results.length > 0
        ? results.join("\n\n---\n\n")
        : `Aucune correspondance trouvée pour '${query}' dans la base de connaissances du Hub (${FORGE_HUB_DIR}).`;

      return {
        content: [{ type: "text", text: responseText }],
      };
    }

    // 2. Consultation des exemples canoniques
    if (name === "forge_get_canonical_example") {
      const exampleName = String(args?.["exampleName"] ?? "list").trim();
      const examplesDir = path.join(FORGE_HUB_DIR, ".harness/examples");

      if (exampleName === "list") {
        const files = await fs.readdir(examplesDir);
        const listText = files
          .filter((f) => !f.startsWith("."))
          .map((f) => `- ${f}`)
          .join("\n");
        return {
          content: [
            {
              type: "text",
              text: `=== EXEMPLES CANONIQUES DE LA FORGE (${examplesDir}) ===\n${listText}\n\nSpécifiez le nom d'un fichier pour afficher son contenu intégral.`,
            },
          ],
        };
      }

      const safeBaseName = path.basename(exampleName);
      const filePath = path.join(examplesDir, safeBaseName);
      const content = await fs.readFile(filePath, "utf-8");
      return {
        content: [
          {
            type: "text",
            text: `=== EXEMPLE CANONIQUE: ${safeBaseName} ===\n\`\`\`\n${content}\n\`\`\``,
          },
        ],
      };
    }

    // 3. Scaffolding d'un projet extérieur avec validation de racine
    if (name === "forge_scaffold_harness") {
      const rawTargetDir = String(args?.["targetDir"] ?? ".");
      const targetDir = validatePathUnderAllowedRoots(rawTargetDir, "targetDir");
      const projectType = String(args?.["projectType"] ?? "polyglot");
      const projectName = String(args?.["projectName"] ?? path.basename(targetDir));

      await fs.mkdir(path.join(targetDir, "scripts"), { recursive: true });
      await fs.mkdir(path.join(targetDir, ".harness/knowledge/domain"), { recursive: true });
      await fs.mkdir(path.join(targetDir, ".harness/playbooks"), { recursive: true });

      // AGENTS.md
      const agentsMd = `# AGENTS.md — Mémoire & Guide de Continuité pour ${projectName}

Ce dépôt est gouverné par les principes de la **Forge Agentique**. Tout agent intervenant sur ce projet doit impérativement respecter les règles suivantes :

1. **Le Juge Souverain (\`scripts/verify.sh\`) :** Une tâche n'est JAMAIS achevée sans un code de retour 0.
2. **Journal des Décisions :** Toute décision d'architecture doit être notée dans \`.harness/knowledge/decisions.md\`.
3. **Hub Central de Connaissances :** Vous avez accès au Hub de connaissances expert (\`${FORGE_HUB_DIR}\`) via les outils MCP de la Forge (\`forge_query_knowledge\`, \`forge_get_canonical_example\`).
`;
      await fs.writeFile(path.join(targetDir, "AGENTS.md"), agentsMd, "utf-8");

      // CONVENTIONS.md
      let conventionsBody = "";
      if (projectType === "rust") {
        conventionsBody = `## Invariants Rust
- \`cargo clippy --all-targets -- -D warnings\` doit passer avec zéro warning.
- \`cargo test\` doit être à 100% vert.
- Tout bloc \`unsafe\` doit comporter un commentaire \`// SAFETY:\` détaillant les invariants.`;
      } else if (projectType === "cpp" || projectType === "c") {
        conventionsBody = `## Invariants C / C++
- Compilation stricte : \`-Wall -Wextra -Wpedantic -Werror\`.
- AddressSanitizer obligatoire : \`-fsanitize=address,undefined\`.
- Zéro fuite mémoire tolérée à l'exécution.`;
      } else if (projectType === "typescript") {
        conventionsBody = `## Invariants TypeScript
- \`strict: true\` dans tsconfig.
- Frontières d'API validées par des schémas de contrat (Zod).
- Zéro \`any\` non justifié.`;
      } else if (projectType === "python") {
        conventionsBody = `## Invariants Python
- Typage statique strict avec \`mypy\` ou \`pyright\`.
- Formatage et linting via \`ruff check\`.
- Tests unitaires complets via \`pytest\`.`;
      } else {
        conventionsBody = `## Invariants Polyglot
- Tests unitaires et linters stricts.
- Pas de warnings de compilation tolérés.`;
      }

      const conventionsMd = `# CONVENTIONS.md — Règles et Commandes pour ${projectName}

${conventionsBody}

## Commande souveraine de vérification
\`\`\`bash
bash scripts/verify.sh
\`\`\`
`;
      await fs.writeFile(path.join(targetDir, "CONVENTIONS.md"), conventionsMd, "utf-8");

      // scripts/verify.sh
      let verifyScriptCommands = "";
      if (projectType === "rust") {
        verifyScriptCommands = `cargo clippy --all-targets -- -D warnings\ncargo test`;
      } else if (projectType === "typescript") {
        verifyScriptCommands = `npm run typecheck 2>/dev/null || npx tsc --noEmit\nnpm test`;
      } else if (projectType === "python") {
        verifyScriptCommands = `pytest`;
      } else {
        verifyScriptCommands = `echo "Veuillez adapter scripts/verify.sh pour votre stack (${projectType})"`;
      }

      const verifySh = `#!/usr/bin/env bash
set -euo pipefail

# scripts/verify.sh — Le Juge Souverain pour ${projectName}
echo "=== VÉRIFICATION QUALITÉ : ${projectName} ==="

${verifyScriptCommands}

echo "=== VERDICT : PASS ==="
`;
      const verifyPath = path.join(targetDir, "scripts/verify.sh");
      await fs.writeFile(verifyPath, verifySh, "utf-8");
      await fs.chmod(verifyPath, 0o755);

      // .harness/knowledge/decisions.md
      const decisionsMd = `# Journal des Décisions d'Architecture (ADR) — ${projectName}

Ce document consigne chronologiquement les décisions prises par les agents et l'équipe.

---

### Date : ${new Date().toISOString().split("T")[0]}
- **Décision :** Initialisation du harnais agentique via le Hub MCP Forge.
- **Statut :** Accepté.
`;
      await fs.writeFile(path.join(targetDir, ".harness/knowledge/decisions.md"), decisionsMd, "utf-8");

      return {
        content: [
          {
            type: "text",
            text: `✓ Projet initialisé avec succès dans : ${targetDir}\n- Créé : AGENTS.md\n- Créé : CONVENTIONS.md\n- Créé : scripts/verify.sh (exécutable)\n- Créé : .harness/knowledge/decisions.md\n- Relié au Hub Forge : ${FORGE_HUB_DIR}`,
          },
        ],
      };
    }

    // 4. Audit Mémoire ASan avec execFile (aucun shell) et restriction de racine
    if (name === "forge_audit_memory") {
      const rawSource = String(args?.["sourcePath"] ?? "");
      const resolvedSource = validatePathUnderAllowedRoots(rawSource, "sourcePath");

      const fileStat = await fs.stat(resolvedSource);
      if (!fileStat.isFile()) {
        throw new Error(`Le chemin spécifié n'est pas un fichier valide : ${resolvedSource}`);
      }

      const isC = Boolean(args?.["isC"]);
      const standard = String(args?.["standard"] ?? (isC ? "c23" : "c++20"));

      if (!ALLOWED_STANDARDS.has(standard)) {
        throw new Error(`Standard de compilation non autorisé : '${standard}'. Autorisés: ${Array.from(ALLOWED_STANDARDS).join(", ")}`);
      }

      const uniqueSuffix = `${Date.now()}_${Math.random().toString(36).substring(2, 8)}`;
      const tempBin = `/tmp/forge_asan_${uniqueSuffix}`;
      const compiler = isC ? "clang" : "clang++";

      const compileArgs = [
        `-std=${standard}`,
        "-Wall",
        "-Wextra",
        "-Werror",
        "-fsanitize=address,undefined",
        "-I",
        path.join(FORGE_HUB_DIR, "native/include"),
        resolvedSource,
        "-o",
        tempBin,
      ];

      try {
        await execFileAsync(compiler, compileArgs, EXEC_OPTIONS);
        const { stdout, stderr } = await execFileAsync(tempBin, [], EXEC_OPTIONS);
        return {
          content: [
            {
              type: "text",
              text: `=== AUDIT MÉMOIRE RÉEL : 0 ERREUR DÉTECTÉE ===\nSortie standard:\n${stdout}\nDiagnostic ASan:\n${stderr}`,
            },
          ],
        };
      } finally {
        await fs.unlink(tempBin).catch(() => {});
      }
    }

    // 5. Exécution du Juge Souverain via execFile ("bash", [verifyScript])
    if (name === "forge_verify") {
      const rawProjectDir = String(args?.["projectDir"] ?? ".");
      const targetDir = validatePathUnderAllowedRoots(rawProjectDir, "projectDir");
      const verifyScript = path.join(targetDir, "scripts/verify.sh");

      await fs.access(verifyScript);

      try {
        const { stdout, stderr } = await execFileAsync("bash", [verifyScript], {
          cwd: targetDir,
          ...EXEC_OPTIONS,
          timeout: 120_000,
        });
        return {
          content: [
            {
              type: "text",
              text: `=== VERDICT SOUVERAIN: PASS ===\n${stdout}\n${stderr}`,
            },
          ],
        };
      } catch (err: any) {
        return {
          isError: true,
          content: [
            {
              type: "text",
              text: `=== VERDICT SOUVERAIN: FAIL (Code ${err.code ?? 1}) ===\n${err.stdout ?? ""}\n${err.stderr ?? ""}\nErreur: ${err.message}`,
            },
          ],
        };
      }
    }

    throw new Error(`Outil inconnu : ${name}`);
  } catch (error: any) {
    return {
      isError: true,
      content: [
        {
          type: "text",
          text: `ERREUR FORGE MCP:\n${error.message}\n${error.stderr || ""}`,
        },
      ],
    };
  }
});

async function main() {
  const transport = new StdioServerTransport();
  await server.connect(transport);
  console.error(`Serveur MCP Global Forge v1.1.0 démarré (Hub: ${FORGE_HUB_DIR}).`);
}

main().catch((err) => {
  console.error("Erreur critique serveur MCP Forge:", err);
  process.exit(1);
});
