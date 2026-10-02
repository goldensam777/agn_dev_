/**
 * @file canonical_branded_contracts.ts
 * @brief Modèle canonique TypeScript (strict + exactOptionalPropertyTypes) :
 *        types brandés + validation Zod aux frontières.
 *
 * Règles illustrées (corpus .harness/knowledge/languages/ts/) :
 *  - core/branded-types.md        : un identifiant n'est pas un string — le brand
 *    rend les permutations d'IDs impossibles À LA COMPILATION, coût zéro à l'exécution.
 *  - core/contracts-validation.md : ZOD SEUL décide de la forme des données qui
 *    franchissent une frontière ; interdiction du `as Type` aveugle.
 *  - quality/anti-patterns.md     : jamais de `any`, jamais de Promise flottante.
 *
 * Typage : tsc --strict --exactOptionalPropertyTypes --noUncheckedIndexedAccess
 */

import { z } from "zod";

// --- 1. Types brandés : la sémantique vit dans le type ---------------------

declare const UserIdBrand: unique symbol;
export type UserId = string & { readonly [UserIdBrand]: true };

declare const ProjectIdBrand: unique symbol;
export type ProjectId = string & { readonly [ProjectIdBrand]: true };

// --- 2. Schémas Zod : SEULE porte d'entrée autorisée (réseau / IPC) --------

export const ProjectMembershipSchema = z.object({
  userId: z.string().uuid(),
  projectId: z.string().uuid(),
  role: z.enum(["viewer", "editor", "admin"]),
  // exactOptionalPropertyTypes : l'optionnel est `T | undefined` de façon explicite.
  invitedBy: z.string().uuid().optional(),
});

export type ProjectMembershipInput = z.input<typeof ProjectMembershipSchema>;
export type ProjectMembership = z.infer<typeof ProjectMembershipSchema>;

// Erreur de frontière : typée, jamais un string nu qui se perd dans les logs.
export class ContractViolationError extends Error {
  // zod v3 : z.ZodIssue[] ; zod v4 : z.core.$ZodIssue[]
  constructor(public readonly issues: z.ZodIssue[]) {
    super("Données entrantes invalides au regard du contrat");
    this.name = "ContractViolationError";
  }
}

// --- 3. Le brand ne se pose QUE sur preuve de validation réussie -----------

export function parseMembership(raw: unknown): ProjectMembership {
  const result = ProjectMembershipSchema.safeParse(raw);
  if (!result.success) {
    throw new ContractViolationError(result.error.issues);
  }
  return result.data;
}

export function toUserId(membership: ProjectMembership): UserId {
  // SEUL `as` toléré du dépôt, et il est justifié : safeParse a tranché juste au-dessus.
  return membership.userId as UserId;
}

// --- 4. API typée : permuter userId/projectId ne compile PAS ---------------

export async function grantAccess(caller: UserId, target: ProjectId): Promise<void> {
  const response = await fetch(`/api/projects/${target}/members`, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ userId: caller }), // UserId → string : licite
  });
  if (!response.ok) {
    throw new Error(`grantAccess a échoué : HTTP ${response.status}`);
  }
}

// --- Usage côté frontière (route serveur / handler IPC) ---------------------
//
//   const membership = parseMembership(req.body); // Zod tranche : 400 ou données sûres
//   const caller = toUserId(membership);          // brand posé, preuve à l'appui
//   await grantAccess(caller, membership.projectId as ProjectId);
