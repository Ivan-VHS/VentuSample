# Beelzebul AppGuard 666 — VentuSample

## Purpose and status

This repository carries the **AppGuard 666 control catalog** alongside its existing RepositoryGuard / resilience policy. The numbering is stable and partitioned into six echelons of 111 positions each. The catalog is a control inventory and conformance contract; it is **not** proof that all 666 protections are implemented or active.

- Catalog state: `policy_catalog_only`
- Runtime enforcement delivered by this catalog change: **none**
- Runtime implementation claims: **0**
- Validator: `scripts/validate_beelzebul_appguard.py`
- Machine-readable catalog: `.beelzebul/appguard.v1.json`
- Schema: `schemas/beelzebul-appguard.v1.schema.json`

## Six echelons

| Echelon | Levels | Responsibility |
|---|---:|---|
| I | L001–L111 | Hardware, kernel, process, memory, thread and runtime foundation |
| II | L112–L222 | Inputs, protocol boundaries, parsing and deserialization |
| III | L223–L333 | Identity, authentication, authorization, sessions and cryptography |
| IV | L334–L444 | Data, state, transactions, caches, queues and consistency |
| V | L445–L555 | Network, integrations, plugins, artifacts and dependencies |
| VI | L556–L666 | Telemetry, abuse controls, degradation, quarantine and correlation |

Each echelon defines eleven subsystem domains. Each domain has ten control positions covering bounded capacity, contract validation, context binding, integrity, concurrency, deadlines, quotas, audit evidence, safe failure and adversarial tests. The 111th position in each echelon is a cross-domain assurance control.

## Repository profile

**Runtime:** C++20/Qt desktop application, offline audio rendering, WAV import/export and built-in plugin host

**Control surfaces**
- WAV and manifest parsing
- audio buffers and DSP worker tasks
- plugin registration and lifecycle
- filesystem import/export
- render quotas and result manifests

**Non-negotiable guardrails**
- Imported audio, manifests and external assets are untrusted inputs
- Audio and plugin processing must retain declared size, frame, time and memory limits
- An in-process plugin host is not a security sandbox; isolate third-party binaries at the OS boundary before claiming isolation

## RepositoryGuard 66 crosswalk

The catalog reserves the 66 repository-rule identifiers:

| Group | IDs | Scope | Candidate AppGuard echelons |
|---|---|---|---|
| A | A-01–A-15 | Atomic source and diff integrity | I, II |
| M | M-01–M-15 | Module boundaries, input contracts and side effects | I, II, IV |
| Y | Y-01–Y-15 | Branch, history, CI and build integrity | V, VI |
| K | K-01–K-15 | Cross-branch and transitive-dependency risk | IV, V, VI |
| O | O-01–O-06 | Shared governance and exception controls | I–VI |

This is a **group-level candidate crosswalk**, not a fabricated per-rule mapping. The current source brief provides the ranges and group-level scope but not the full text of every rule. Exact rule-to-`Lxxx` mappings remain pending until the canonical 66 rule definitions are linked. The validator ensures all 66 IDs are present and prevents this pending status from being silently represented as complete.

## Verdict and event contract

Permitted verdicts:

- `allow` — the relevant control passed.
- `quarantine` — isolate the object or workflow pending review.
- `deny` — reject the operation at the responsible trust boundary.
- `degrade` — use a documented safe, limited operating mode.

Every event must identify `level_id`, `rule_id`, `object_ref`, `reason`, `verdict`, `timestamp` and `evidence_ref`. Exceptions require a named owner, rationale, expiry time and approval evidence. Secrets and raw credentials must never be copied into audit events.

## Activation and acceptance sequence

1. Link each of the 66 repository rules to its exact source text and one or more AppGuard controls.
2. For each `Lxxx`, identify the real enforcement point in this repository or its hosting OS/runtime.
3. Implement the control at that enforcement point; do not substitute documentation, a UI indicator or a catalog entry for enforcement.
4. Add positive, negative, boundary and fuzz/regression tests.
5. Attach source, test and CI evidence before promoting a level from `planned` to `partial`, `implemented` or `verified`.
6. Enable runtime controls in observe-only mode first where safe; enforce deny/quarantine only at explicit trust boundaries.
7. Verify safe fallback, rollback and operator-review paths before production activation.

A level is `implemented` only when executable enforcement and tests exist. A level is `verified` only when verification evidence is also linked. The catalog validator enforces these evidence requirements but does not perform security testing of the application.

## CI scenario

The workflow `.github/workflows/beelzebul-appguard.yml` checks:

- exact ranges and continuity from L001 through L666;
- six echelons with 111 positions each;
- 11 subsystem domains and 10 controls per domain, plus one assurance control;
- all 66 RepositoryGuard rule IDs;
- valid candidate crosswalk metadata;
- stable verdict and exception contracts;
- consistency between implementation status, enforcement and evidence;
- regression tests for missing levels, duplicate identifiers, range errors and false implementation claims.

Run locally:

```sh
python scripts/validate_beelzebul_appguard.py
python -m unittest discover -s tests -p "test_beelzebul_appguard.py" -v
```

A passing CI result means the **catalog contract** is internally consistent. It does not mean 666 runtime controls are active.
