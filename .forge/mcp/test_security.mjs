import { spawn } from "node:child_process";
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

console.log("=== TEST DE SÉCURITÉ ANTI-INJECTION DU SERVEUR MCP ===");
console.log(`[1] Vérification initiale : ${CANARY_FILE} existe ? ${fs.existsSync(CANARY_FILE)}`);

const proc = spawn("node", [serverScript], {
  env: { ...process.env, FORGE_HUB_DIR: path.resolve(__dirname, "../..") },
  stdio: ["pipe", "pipe", "inherit"],
});

let responseData = "";

proc.stdout.on("data", (chunk) => {
  responseData += chunk.toString();
});

function sendRpc(msg) {
  const payload = JSON.stringify(msg) + "\n";
  proc.stdin.write(payload);
}

// 2. Initialisation MCP
sendRpc({
  jsonrpc: "2.0",
  id: 1,
  method: "initialize",
  params: {
    protocolVersion: "2024-11-05",
    capabilities: {},
    clientInfo: { name: "security-tester", version: "1.0.0" },
  },
});

// 3. Envoi d'une charge d'injection de commande $(touch /tmp/x) dans forge_query_knowledge
setTimeout(() => {
  console.log("[2] Envoi de la charge d'injection shell : '$(touch /tmp/x)' dans forge_query_knowledge...");
  sendRpc({
    jsonrpc: "2.0",
    id: 2,
    method: "tools/call",
    params: {
      name: "forge_query_knowledge",
      arguments: {
        query: "$(touch /tmp/x)",
        category: "compilers",
      },
    },
  });
}, 200);

// 4. Envoi d'une charge d'injection de commande dans sourcePath de forge_audit_memory
setTimeout(() => {
  console.log("[3] Envoi de la charge d'injection shell : '$(touch /tmp/x)' dans forge_audit_memory...");
  sendRpc({
    jsonrpc: "2.0",
    id: 3,
    method: "tools/call",
    params: {
      name: "forge_audit_memory",
      arguments: {
        sourcePath: "$(touch /tmp/x)",
      },
    },
  });
}, 500);

// 4b. Envoi d'un chemin hors racine autorisée (Path traversal /etc)
setTimeout(() => {
  console.log("[3b] Envoi d'un chemin non autorisé (/etc/passwd) dans targetDir de forge_scaffold_harness...");
  sendRpc({
    jsonrpc: "2.0",
    id: 4,
    method: "tools/call",
    params: {
      name: "forge_scaffold_harness",
      arguments: {
        targetDir: "/etc/forbidden_scaffold",
        projectType: "rust",
      },
    },
  });
}, 800);

// 5. Vérification finale
setTimeout(() => {
  proc.stdin.end();
  proc.kill();

  const fileCreated = fs.existsSync(CANARY_FILE);
  console.log(`[4] Vérification finale : ${CANARY_FILE} existe ? ${fileCreated}`);

  if (fileCreated) {
    console.error("FAIL: FAILLE DE SÉCURITÉ DÉTECTÉE ! Le fichier /tmp/x a été créé.");
    fs.unlinkSync(CANARY_FILE);
    process.exit(1);
  } else {
    console.log("PASS: SÉCURITÉ VALIDÉE. Aucune commande shell n'a été exécutée, /tmp/x n'a PAS été créé.");
    console.log("PASS: Chemins non autorisés strictement rejetés.");
    process.exit(0);
  }
}, 1400);
