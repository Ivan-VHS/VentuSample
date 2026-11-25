import test from "node:test";
import assert from "node:assert/strict";
import { readFileSync } from "node:fs";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";
import { validateProjectLadder } from "../tools/validate_project_ladder.mjs";

const ROOT = resolve(dirname(fileURLToPath(import.meta.url)), "..");
const record = JSON.parse(readFileSync(resolve(ROOT, ".github/project-ladder.v1.json"), "utf8"));

test("the repository maturity record satisfies cumulative gates and evidence paths", () => {
  assert.deepEqual(validateProjectLadder(record, ROOT), []);
});

test("stage names and order are canonical", () => {
  const changed = structuredClone(record);
  changed.stages[2].name = "Vanguard";
  assert.ok(validateProjectLadder(changed, ROOT).some(error => error.includes("canonical order")));
});

test("a completed stage cannot follow a pending stage", () => {
  const changed = structuredClone(record);
  changed.stages[0].status = "pending";
  assert.ok(validateProjectLadder(changed, ROOT).some(error => error.includes("completed stage cannot follow")));
});

test("missing evidence paths are rejected", () => {
  const changed = structuredClone(record);
  const stage = changed.stages.find(item => item.number === changed.current_stage);
  stage.evidence[0].path = "this-path-must-not-exist";
  assert.ok(validateProjectLadder(changed, ROOT).some(error => error.includes("does not exist")));
});

test("a completed stage cannot omit one of its required evidence categories", () => {
  const changed = structuredClone(record);
  const stage = changed.stages.find(item => item.number === 3);
  stage.evidence = stage.evidence.filter(item => item.kind !== "tests");
  assert.ok(validateProjectLadder(changed, ROOT).some(error => error.includes("missing required evidence kind tests")));
});

test("verified status requires a pinned CI run and test evidence", () => {
  const changed = structuredClone(record);
  changed.assessment = "verified";
  assert.ok(validateProjectLadder(changed, ROOT).some(error => error.includes("verified assessment requires")));
});
