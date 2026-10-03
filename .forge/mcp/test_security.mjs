import { Client } from "@modelcontextprotocol/sdk/client/index.js";
import { StdioClientTransport } from "@modelcontextprotocol/sdk/client/stdio.js";
import fs from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const serverScript = path.join(__dirname, "dist/index.js");

const CANARY_FILE = "/tmp/x";

// 1. S'assurer que le fichier témoin /tmp/x n'existe pas avant le test
if (fs.existsSync(CANARY_FILE)) {
  fs.unlinkSync(CANARY_FILE);
}

console.log("=== TEST DE SÉCURITÉ ANTI-INJECTION & ARGUMENTS DU SERVEUR MCP ===");
console.log(`[1] Vérification initiale : ${CANARY_FILE} existe ? ${fs.existsSync(CANARY_FILE)}`);

async function runSecurityTests() {
  const transport = new StdioClientTransport({
    command: "node",
    args: [serverScript],
    env: { ...process.env, FORGE_HUB_DIR: path.resolve(__dirname, "../..") },
  });

  const client = new Client(
    { name: "security-tester", version: "1.2.0" },
    { capabilities: {} }
  );

  await client.connect(transport);

  // Test 1 : Injection de commande shell $(touch /tmp/x)
  console.log("[2] Envoi de la charge d'injection shell : '$(touch /tmp/x)' dans forge_query_knowledge...");
  const resInjection = await client.callTool({
    name: "forge_query_knowledge",
    arguments: {
      query: "$(touch /tmp/x)",
      category: "compilers",
    },
  });

  // Test 2 : Argument CLI sensible '--help'
  console.log("[3] Envoi de la requête '--help' dans forge_query_knowledge (-e et -- requis pour grep)...");
  const resHelp = await client.callTool({
    name: "forge_query_knowledge",
    arguments: {
      query: "--help",
      category: "all",
    },
  });

  // Test 3 : Rejet de chemin hors racine autorisée (/tmp non sandboxé)
  console.log("[4] Test de rejet de chemin /tmp non sandboxé dans forge_scaffold_harness...");
  const resTraversal = await client.callTool({
    name: "forge_scaffold_harness",
    arguments: {
      targetDir: "/tmp/unauthorized_external_dir",
      projectType: "rust",
    },
  });

  await client.close();

  const fileCreated = fs.existsSync(CANARY_FILE);
  const helpHandled = resHelp && !resHelp.isError && Array.isArray(resHelp.content);
  const traversalRejected = Boolean(resTraversal.isError);

  console.log(`\n=== RÉSULTATS DES CONTRÔLES DE SÉCURITÉ ===`);
  console.log(`[Vérif 1] Fichier témoin ${CANARY_FILE} créé ? ${fileCreated} (Attendu : false)`);
  console.log(`[Vérif 2] Requête '--help' traitée sans erreur CLI ? ${helpHandled} (Attendu : true)`);
  console.log(`[Vérif 3] Accès /tmp non sandboxé rejeté ? ${traversalRejected} (Attendu : true)`);

  if (fileCreated) {
    console.error("FAIL: FAILLE DE SÉCURITÉ ! Le fichier /tmp/x a été créé.");
    fs.unlinkSync(CANARY_FILE);
    process.exit(1);
  }

  if (!helpHandled) {
    console.error("FAIL: La requête '--help' n'a pas été traitée correctement.");
    process.exit(1);
  }

  if (!traversalRejected) {
    console.error("FAIL: Le chemin /tmp hors sandbox n'a pas été rejeté.");
    process.exit(1);
  }

  console.log("\n✓ TOUS LES TESTS DE SÉCURITÉ ET D'ARGUMENTS MCP ONT RÉUSSI !");
}

runSecurityTests().catch((err) => {
  console.error("Erreur critique test de sécurité:", err);
  process.exit(1);
});
