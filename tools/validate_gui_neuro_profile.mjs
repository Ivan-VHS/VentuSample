#!/usr/bin/env node
import { appendFileSync, existsSync, readFileSync } from "node:fs";
import { execFileSync } from "node:child_process";
import { resolve } from "node:path";

const root = process.cwd();
const profilePath = resolve(root, "gui-neuro-profile.v1.json");
const fail = (message) => {
  console.error("GUI neuro-profile validation failed: " + message);
  process.exit(1);
};

let profile;
try {
  profile = JSON.parse(readFileSync(profilePath, "utf8"));
} catch (error) {
  fail("cannot parse gui-neuro-profile.v1.json: " + error.message);
}

if (profile.schema !== "dak.gui-neuro-profile.v1" || profile.version !== 1)
  fail("unsupported profile schema or version");

const expectedRepo = process.env.GITHUB_REPOSITORY;
if (expectedRepo && profile.repository !== expectedRepo)
  fail("profile repository does not match GITHUB_REPOSITORY");

if (!Array.isArray(profile.ui_stack) || profile.ui_stack.length === 0)
  fail("ui_stack must be a non-empty array");
if (!Array.isArray(profile.primary_ui_surfaces) || profile.primary_ui_surfaces.length === 0)
  fail("primary_ui_surfaces must be a non-empty array");

for (const item of profile.primary_ui_surfaces) {
  if (typeof item !== "string" || item.startsWith("/") ||
      item.includes("..") || item.includes(String.fromCharCode(92)))
    fail("unsafe primary UI surface: " + String(item));
  if (!existsSync(resolve(root, item)))
    fail("declared primary UI surface does not exist: " + item);
}

const automation = profile.automation ?? {};
if (automation.agent_mode !== "supervised-neuro-agent")
  fail("agent_mode must remain supervised");
if (automation.target_branch_policy !== "feature-branch-only")
  fail("changes must use feature branches");
if (automation.human_review_required !== true ||
    automation.allow_main_branch_writes !== false ||
    automation.allow_auto_merge !== false)
  fail("human review, protected main and disabled auto-merge are mandatory");
if (typeof automation.model_runtime !== "string" ||
    !automation.model_runtime.includes("not invoked by this workflow"))
  fail("this workflow must not claim to run an unconfigured model");

const privacy = profile.privacy ?? {};
if (privacy.external_model_data !== "deny_by_default" ||
    privacy.telemetry !== "disabled" ||
    privacy.secret_access !== "deny")
  fail("privacy defaults must deny external data transfer, telemetry and secrets");

const normalizeRules = (rules, label) => {
  if (!Array.isArray(rules) || rules.length === 0) fail(label + " must be a non-empty array");
  for (const rule of rules) {
    if (typeof rule !== "string" || rule.length === 0 || rule.startsWith("/") ||
        rule.includes("..") || rule.includes(String.fromCharCode(92)))
      fail("unsafe " + label + " entry: " + String(rule));
  }
  return rules;
};

const allowed = normalizeRules(profile.allowed_write_paths, "allowed_write_paths");
const protectedPaths = normalizeRules(profile.protected_write_paths, "protected_write_paths");
const matches = (file, rule) => rule.endsWith("/")
  ? file.startsWith(rule)
  : file === rule;

const baseRef = process.env.GITHUB_BASE_REF;
const headRef = process.env.GITHUB_HEAD_REF;
if (headRef && ["main", "master"].includes(headRef))
  fail("direct main/master branch changes are prohibited");

let changed = [];
if (baseRef) {
  try {
    const raw = execFileSync("git", ["diff", "--name-only", "-z", "origin/" + baseRef + "...HEAD"], {
      cwd: root, encoding: "utf8", stdio: ["ignore", "pipe", "pipe"],
    });
    changed = raw.split(String.fromCharCode(0)).filter(Boolean);
  } catch (error) {
    fail("cannot calculate PR diff safely: " + error.message);
  }
  if (changed.length === 0) fail("PR contains no changed files for the neuro-GUI profile gate");

  const rejected = [];
  for (const file of changed) {
    const denied = protectedPaths.some((rule) => matches(file, rule));
    const permitted = allowed.some((rule) => matches(file, rule));
    if (denied || !permitted) rejected.push(file);
  }
  if (rejected.length) fail("changes outside the approved GUI scope: " + rejected.join(", "));
}

const summary = [
  "### GUI neuro-autoimprovement profile",
  "",
  "- Repository: " + profile.repository,
  "- UI stack: " + profile.ui_stack.join(", "),
  "- Primary UI surfaces verified: " + profile.primary_ui_surfaces.length,
  "- Changed files checked: " + changed.length,
  "- Human review required: yes",
  "- Direct main writes / auto-merge: disabled",
  "- External model data / telemetry / secrets: denied by default",
  "",
  "This gate validates scope and safety policy; it does not execute or train a neural model.",
  "",
].join(String.fromCharCode(10));

if (process.env.GITHUB_STEP_SUMMARY) {
  try { appendFileSync(process.env.GITHUB_STEP_SUMMARY, summary, "utf8"); } catch {}
}
console.log("GUI neuro-profile is valid. Scope and autonomy guardrails are enforced.");
