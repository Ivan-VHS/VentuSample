# Defensive security doctrine and BAV context taxonomy

**Policy:** `duckos.bav.defense-context.v1`  
**Catalog:** [`security/defense-context.v1.tsv`](../security/defense-context.v1.tsv)  
**Status:** context taxonomy is defined; full antivirus coverage is not implied.

## Purpose

This policy carries three groups of user-supplied Russian-language terms into repository security governance and DuckOS BAV design:

| Category ID | Context family | Terms |
|---|---|---:|
| `wmd_cbrn` | Nuclear, atomic, thermonuclear and other WMD references | 20 |
| `chinese_military_platforms` | References to Chinese military vehicles, aircraft, naval platforms and equipment | 20 |
| `air_defense_systems` | References to air-defence systems and portable air-defence equipment | 20 |

The TSV catalog is deliberately a metadata vocabulary. It contains no weapon specifications, construction information, targeting data or operating instructions.

## Non-negotiable distinction: context is not malware

A phrase match means only that a document or selected file discusses a listed subject. It is **not** evidence of malware, malicious intent, an export-control violation or an unsafe executable.

BAV and repository scanners MUST keep these outcomes separate:

- **Malware verdict:** based on independent technical evidence such as verified malicious signatures, exploit structure, unsafe behavior, tampering, invalid provenance or a confirmed compromise.
- **Sensitive-context tags:** descriptive labels returned by this catalog.
- **Human review:** required when the surrounding context, ownership, provenance or intended handling is unclear.

A context match MUST NOT increase the malware risk score, set `blocked`, trigger quarantine, delete a file, prevent a build, or revoke a capability by itself. Do not infer nationality, intent or threat level from a phrase.

## Defensive handling rules

1. **Read-only inspection by default.** Inspect bounded metadata and content without executing, loading, installing, unpacking or modifying the inspected object.
2. **Local-first privacy.** Do not upload file contents, paths, snippets, device identifiers or reports to a remote service by default. Any later sharing path needs a separate, explicit approval and redaction step.
3. **Bounded parsers.** Enforce file-size, total-byte, line-length, archive-depth, decompression-ratio, recursion and time limits. Reject path traversal, symlink escape, malformed encodings and duplicate manifest keys.
4. **Provenance before trust.** Verify hashes, signatures, signer identity, certificate policy, version, source and release provenance independently. A matching hash establishes identity, not authorship or safety.
5. **Least privilege.** BAV runs unprivileged for ordinary inspection. A privileged native broker is a separate component with narrow capabilities, short-lived grants, audit events and explicit approval.
6. **No implicit execution.** Never execute samples, scripts, macros, installers, driver payloads, firmware or manifest-provided entry points as part of scanning.
7. **Explainable decisions.** Reports include policy version, scanner version, source type, evidence identifiers and a concise reason. Do not retain matched source snippets or secret values.
8. **Reversible response.** Quarantine is allowed only after an independent malware verdict and a separately defined response policy. It must preserve a recoverable original and an audit trail; context-only matches remain informational.
9. **Fail safely.** Parser errors, timeouts, missing trust roots and unsupported formats produce `review` or `unknown`, never a fabricated `clear`.
10. **Supply-chain integrity.** Pin dependencies and CI actions, generate an SBOM, review transitive dependencies, sign release artifacts and verify the signature before installation.

## BAV integration boundary in DuckOS

The first BAV increment is a small Java context-classification library. It loads the TSV catalog, applies Unicode NFKC normalization, uses whole-phrase boundaries, rejects oversized input and returns category/term IDs only. It does not scan binaries, inspect process memory, verify signatures or detect malware.

The full BAV architecture must keep these components separate:

- **Content/context classifier:** informational tags from this catalog.
- **File-format and archive inspectors:** bounded, read-only parsers.
- **Integrity and provenance verifier:** cryptographic hash and signature checks with an explicit trust store.
- **Malware analyzers:** independent static/behavioral evidence providers, isolated from the UI.
- **Policy engine:** combines evidence without allowing context tags to become malware verdicts.
- **Native response broker:** explicit, capability-scoped and auditable actions; no browser-shell privilege.

Mora/Sentinel preferences are not OS enforcement. Native process isolation, file access control, driver trust and quarantine require a real DuckOS service and kernel/broker enforcement.

## Repository-specific application

- **DuckOS:** BAV context tags are local metadata; Sentinel and the native security broker remain separate enforcement layers.
- **Reaper:** keep the 66-rule repository baseline intact; treat the catalog as text data and validate it as part of policy governance.
- **OPlague:** driver/INF/ZIP/PE metadata is evidence, not an authorization to install or load a driver. The project remains read-only.
- **Pytyliki:** hardware and source inspection stays advisory and bounded; no firmware flashing, device writes or privileged operations are added.
- **VentuSample:** audio manifests, presets and plugin metadata remain data-only; the catalog does not authorize dynamic library or plugin loading.

## Verification requirements

The Java context module has a dependency-free self-test for all three categories, ordinary-text non-matches, whole-phrase boundaries and input-size limits. Run it from the repository root:

```sh
mkdir -p build/bav-context
javac --release 17 -d build/bav-context \
  bav/java/duckos/bav/context/ContextClassifier.java \
  bav/java/duckos/bav/context/ContextClassifierSelfTest.java
java -ea -cp build/bav-context duckos.bav.context.ContextClassifierSelfTest \
  security/defense-context.v1.tsv
```

This test validates the context classifier only. It is not a malware-detection benchmark, a sandbox certification or proof of full antivirus protection.
