#!/usr/bin/env node
import { readFile } from "node:fs/promises";
import { resolve } from "node:path";
import { fileURLToPath } from "node:url";

const root = resolve(fileURLToPath(new URL("..", import.meta.url)));
const fail = (message) => { console.error("i18n: " + message); process.exitCode = 1; };
const readJson = async (path) => JSON.parse(await readFile(resolve(root, path), "utf8"));
const record = (value) => value !== null && typeof value === "object" && !Array.isArray(value);

try {
  const manifest = await readJson("i18n/manifest.v1.json");
  const set = await readJson("i18n/catalogs.v1.json");
  if (manifest.schema !== "dak.i18n-manifest/v1" || set.schema !== "dak.i18n-catalog-set/v1") fail("unsupported schema");
  if (manifest.sourceLocale !== set.sourceLocale) fail("source locale mismatch");
  if (manifest.languageTags?.acceptAnyWellFormedTag !== true) fail("open BCP-47 policy is required");
  if (!Array.isArray(manifest.fallback) || manifest.fallback.at(-1) !== manifest.sourceLocale) fail("fallback must end at source locale");
  const source = set.locales?.[manifest.sourceLocale];
  if (!record(source?.messages) || Object.keys(source.messages).length === 0) fail("source messages are missing");
  const sourceKeys = Object.keys(source?.messages ?? {}).sort();

  const canonical = (tag) => Intl.getCanonicalLocales(tag)[0];
  for (const [tag, meta] of Object.entries(manifest.catalogs ?? {})) {
    try { if (canonical(tag) !== tag) fail("non-canonical locale tag: " + tag); }
    catch { fail("invalid BCP-47 locale tag: " + tag); continue; }
    const bundle = set.locales?.[tag];
    if (!record(meta) || !record(bundle?.messages)) { fail("catalog metadata/messages missing: " + tag); continue; }
    if (!bundle.name || !["ltr","rtl"].includes(bundle.direction)) fail("invalid catalog metadata: " + tag);
    for (const [key, value] of Object.entries(bundle.messages)) {
      if (typeof value !== "string" || !value.trim() || value.includes("\uFFFD")) fail("invalid message: " + tag + ":" + key);
    }
    if (meta.status === "complete") {
      const keys = Object.keys(bundle.messages).sort();
      if (sourceKeys.join("\0") !== keys.join("\0")) fail("complete catalog key mismatch: " + tag);
    }
    if (tag === manifest.sourceLocale && meta.status !== "source") fail("source catalog must be marked source");
  }
  for (const tag of Object.keys(set.locales ?? {})) {
    try { canonical(tag); } catch { fail("invalid catalog tag: " + tag); }
    if (!Object.hasOwn(manifest.catalogs ?? {}, tag)) fail("unregistered catalog: " + tag);
  }
  if (typeof Intl.Segmenter !== "function") fail("Intl.Segmenter is required for grapheme-aware editing");
  for (const tag of Object.keys(manifest.catalogs ?? {})) {
    try {
      new Intl.NumberFormat(tag).format(12345.67);
      new Intl.DateTimeFormat(tag).format(new Date("2026-01-02T03:04:05Z"));
      new Intl.PluralRules(tag).select(2);
    } catch (error) { fail("Intl failed for " + tag + ": " + error.message); }
  }
  if (process.exitCode) process.exit(process.exitCode);
  console.log("i18n: " + manifest.project + " passed; " + Object.keys(manifest.catalogs).length + " catalogs, " + sourceKeys.length + " message ids.");
} catch (error) { fail(error.stack ?? String(error)); }
