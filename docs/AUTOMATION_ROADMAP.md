# Automation Roadmap — VentuSample

**Contract:** `anatidae.automation-roadmap.v1`  
**Repository:** `Ivan-VHS/VentuSample`  
**Default branch:** `main`

## Scope

Deterministic sample generation, bounded offline DSP, PCM16 WAV export, and versioned sample-pack manifests.

Stack: C++20, Qt 6, CMake.

This roadmap turns the 20 requested directions into traceable workstreams. It distinguishes existing automation from work that still needs to be built: **active** means a relevant control already exists; **extend** means a control exists but does not yet deliver the full target; **planned** means it is not enabled and must not be described as implemented.

## Execution phases

1. **Baseline and contract.** Keep the roadmap, schema, documentation, and evidence references synchronized. The contract workflow checks the registry on pull requests, relevant pushes, a weekly schedule, and manual dispatch.
2. **Continuous verification.** Extend existing CI, architecture, quality, branch-synchronization, and verified auto-merge controls without duplicating them or weakening required checks.
3. **Recovery, scale, and release gates.** Introduce bounded recovery and resource-aware validation only after tests. Deployment and publication remain disabled until a protected target, rollback procedure, and human approval are defined.

## Workstream register

| ID | Direction | Priority | Status | Current evidence |
|---|---|---:|---|---|
| `01-plan-governance` | Запланировать | P0 | extend | `project-contract.json`, `docs/DOCUMENTATION_CONTROL.md` |
| `02-organize-triggers` | Организовать | P0 | active | `.github/workflows/verified-auto-merge.yml`, `.github/workflows/update-behind-pr-branches.yml` |
| `03-systematize-sync` | Систематизировать | P0 | active | `project-contract.json`, `federation/octa-pyramid.v1.json` |
| `04-prioritize-change-flow` | Приоритизировать | P1 | extend | `.github/workflows/improvement-conflict.yml`, `.github/workflows/quality-remediation.yml` |
| `05-refresh-project-config` | Актуализировать | P0 | active | `project-contract.json`, `.architecture/resilience-policy.json` |
| `06-optimize-toolchain` | Оптимизировать | P1 | extend | `.architecture/resilience-policy.json`, `.architecture/resilience-policy.schema.json` |
| `07-synchronize-branches` | Синхронизировать | P0 | active | `.github/workflows/update-behind-pr-branches.yml` |
| `08-integrate-projects` | Интегрировать | P0 | active | `project-contract.json`, `federation/octa-pyramid.v1.json` |
| `09-modernize-quality-flow` | Модернизировать | P0 | active | `.github/workflows/quality-remediation.yml`, `quality-remediation.v1.json` |
| `10-automate-diagnostics` | Автоматизировать | P0 | active | `.github/workflows/quality-remediation.yml`, `.github/workflows/update-behind-pr-branches.yml` |
| `11-structure-test-updates` | Структурировать | P0 | active | `.github/workflows/quality-repeatability-native.yml`, `tests/test_generator.cpp` |
| `12-scale-validation` | Масштабировать | P1 | extend | `.github/workflows/quality-repeatability-native.yml`, `quality-repeatability-native.v1.json` |
| `13-implement-verification` | Реализовать | P0 | active | `.github/workflows/quality-commercial.yml`, `.github/workflows/quality-remediation.yml` |
| `14-coordinate-deployment` | Координировать | P2 | planned | `.github/workflows/verified-auto-merge.yml`, `project-contract.json` |
| `15-consolidate-test-matrix` | Консолидировать | P1 | active | `.github/workflows/quality-repeatability-native.yml`, `tests/test_generator.cpp` |
| `16-improve-recovery` | Усовершенствовать | P1 | planned | `.architecture/resilience-policy.json`, `.architecture/resilience-policy.schema.json` |
| `17-test-controls` | Протестировать | P0 | active | `.github/workflows/quality-remediation.yml`, `tests/test_generator.cpp` |
| `18-verify-observability` | Верифицировать | P1 | extend | `.github/workflows/verified-auto-merge.yml`, `.github/workflows/update-behind-pr-branches.yml` |
| `19-document-automation` | Документировать | P1 | extend | `docs/README.md`, `docs/DOCUMENTATION_CONTROL.md` |
| `20-release-publication` | Развернуть | P2 | planned | `.github/workflows/verified-auto-merge.yml`, `project-contract.json` |

## Operating rules

- The new roadmap workflow is read-only and uses `contents: read`.
- The roadmap validator is also run in the existing CI workflow, which must finish successfully before the verified auto-merge workflow acts.
- Existing verified auto-merge remains conditional on successful checks and repository protection; this roadmap does not bypass either.
- Branch synchronization must use an expected head SHA, operate only on eligible same-repository pull requests, and leave conflicts for maintainers.
- Diagnostics, schema checks, and scheduled audits must be deterministic, bounded, and non-destructive.
- Automated code correction may produce a reviewable proposal; it must not silently rewrite source or commit directly to `main`.
- This roadmap does **not** authorize production deployment, external publication, secret rotation, destructive cleanup, firmware flashing, or driver installation.
- A workstream marked `planned` or `extend` is not complete. Update its status only with implementation and passing evidence.

## Validation

Run locally:

```bash
python scripts/validate_automation_roadmap.py --strict
```

The validator uses only the Python standard library and checks all 20 identifiers, project metadata, safety settings, documentation coverage, and the existence of every referenced evidence file.
