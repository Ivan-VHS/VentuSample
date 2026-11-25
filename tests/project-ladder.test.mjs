import test from "node:test";
import assert from "node:assert/strict";
import { readFileSync } from "node:fs";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";
import { validateProjectLadder } from "../tools/validate_project_ladder.mjs";
const ROOT=resolve(dirname(fileURLToPath(import.meta.url)),"..");
const record=JSON.parse(readFileSync(resolve(ROOT,".github/project-ladder.v1.json"),"utf8"));

test("maturity record satisfies cumulative gates and evidence paths",()=>assert.deepEqual(validateProjectLadder(record,ROOT),[]));
test("stage names and order are canonical",()=>{const x=structuredClone(record);x.stages[2].name="Vanguard";assert.ok(validateProjectLadder(x,ROOT).some(e=>e.includes("canonical order")));});
test("completed stages cannot skip a pending gate",()=>{const x=structuredClone(record);x.stages[0].status="pending";assert.ok(validateProjectLadder(x,ROOT).some(e=>e.includes("completed stage cannot follow")));});
test("non-existent evidence paths are rejected",()=>{const x=structuredClone(record);x.stages[x.current_stage-1].evidence[0].path="missing-evidence-file";assert.ok(validateProjectLadder(x,ROOT).some(e=>e.includes("does not exist")));});
test("required evidence categories cannot be omitted",()=>{const x=structuredClone(record);const s=x.stages[x.current_stage-1];s.evidence=s.evidence.filter(e=>e.kind!=="tests");assert.ok(validateProjectLadder(x,ROOT).some(e=>e.includes("missing required evidence kind")));});
test("verified status requires a pinned CI run and test evidence",()=>{const x=structuredClone(record);x.assessment="verified";assert.ok(validateProjectLadder(x,ROOT).some(e=>e.includes("verified assessment requires")));});
