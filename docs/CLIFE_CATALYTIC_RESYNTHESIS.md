# CLife Catalytic Resynthesis — VentuSample

**Contract:** [resynthesis-policy.v1.json](../.clife/resynthesis-policy.v1.json)  
**Schema:** [clife-resynthesis.v1.schema.json](../schemas/clife-resynthesis.v1.schema.json)  
**Validator and Git recovery CLI:** [validate_clife_resynthesis.py](../tools/validate_clife_resynthesis.py)  
**Tests:** [test_clife_resynthesis.py](../tests/test_clife_resynthesis.py)

## Purpose

CLife applies the catalytic-converter analogy to controlled repository evolution. It is a change-governance and recovery mechanism, not an autonomous source rewriter. It protects the known-good core, introduces improvements through explicit extension points, tests each transformation and preserves a route to a previous state.

Repository focus: **C++20 offline audio/DSP engine, Qt desktop integration, deterministic rendering and versioned media packs.**

## Mapping from catalyst to software

| Catalytic element | CLife repository equivalent | Operating rule |
|---|---|---|
| Monolith substrate | Core source, public interfaces, ABI/API, build system and reference tests | Preserve invariants; treat core changes as high-impact migrations |
| Porous washcoat | Tools, adapters, modules, plugins, schemas and integration points | Extend through narrow, versioned boundaries rather than coupling extensions to the core |
| Active catalytic sites | Small, reviewable transformations with explicit acceptance criteria | A change is accepted only after relevant checks pass |
| Oxygen storage capacity (OSC) | Annotated Git checkpoint tag and recovery branch | Checkpoint only a clean worktree; recover by creating a new branch |
| Lambda operating window | Baseline + tests + compatibility + security review | No single quality signal can compensate for a failed mandatory gate |
| Converter diagnostics | CI results, Git metadata, interface checks, dependency provenance and regression results | Collect evidence without copying source contents or secrets |
| Mega projection | 30/90/180/365-day trend review | Forecast only from measured data; label estimates and unknowns |

## Project-specific boundaries

Audio rendering remains offline; imported media, plugin metadata and manifests remain bounded, validated and provenance-aware.

src/, CMakeLists.txt and tests/ preserve core DSP and rendering contracts; adapters and pack extensions remain isolated.

The path mapping is recorded in the machine-readable policy. A path being designated as protected does not prohibit reviewed changes; it means changes require deliberate review and the relevant regression gates.

## Reversible resynthesis lifecycle

1. **Inventory:** identify the affected modules, owners, public contracts and dependencies.
2. **Baseline:** run the existing build, unit, integration and contract checks; save their actual results.
3. **Checkpoint:** commit the intended baseline and create a local annotated tag from a clean worktree.
4. **Isolate:** work on a feature branch and constrain the change surface.
5. **Transform:** apply one coherent change at a time; do not rewrite unrelated code.
6. **Validate:** run the repository's existing CI and targeted regression, security and compatibility checks.
7. **Compare:** assess behavior and measured quality against the baseline.
8. **Accept or recover:** accept through human review, or create a new branch from the checkpoint.

## Commands

Run from the repository root:

```bash
python tools/validate_clife_resynthesis.py --strict --audit
python -m unittest discover -s tests -p "test_clife_resynthesis.py" -v
```

After committing all intended work and confirming that the worktree is clean:

```bash
python tools/validate_clife_resynthesis.py --checkpoint before-refactor
```

The command prints a tag such as `clife/checkpoint-before-refactor-YYYYMMDDTHHMMSSZ`. Keep the exact tag name.

To preserve that state as a separate recovery line without resetting the current branch:

```bash
python tools/validate_clife_resynthesis.py --resurrect clife/checkpoint-before-refactor-YYYYMMDDTHHMMSSZ clife/recovery-before-refactor
```

The recovery operation creates a new local branch and does not switch branches, overwrite an existing branch, push to a remote, or alter the working tree. Push and merge remain explicit human decisions.

## Diagnostics and evidence

The audit reads only repository metadata: current branch, commit identifier, clean/dirty state, change-entry count and tracked-file count. It does not inspect file contents, credentials, device state or personal data.

CI validates the policy and runs recovery tests. Existing project tests remain the source of truth for product behavior. Dependency scanning, release SBOM/provenance, ABI/API drift metrics and long-term reliability trends must be reported only where existing tools produce evidence; this initial CLife rollout does not claim those measurements have already been collected.

## Mega projection and maturity

The configured review horizons are 30, 90, 180 and 365 days. Initial status is deliberately **pending initial measurement**. At each review, record the observation window, sample size, source, baseline, result and uncertainty. Do not extrapolate a reliability forecast from a single CI run or from unmeasured indicators.

The current adoption status is **pilot**: the contract, validation, audit and local recovery controls are introduced, while project-wide quality trends still need to accumulate.
