# Roadmap governance and bounded CI scale

**Repository:** `Ivan-VHS/VentuSample`  
**Policies:** `.architecture/roadmap-governance.v1.json`, `.architecture/ci-scale-policy.v1.json`  
**Validator:** `tools/validate_delivery_contracts.py`  
**Product boundary:** offline sample generation, bounded WAV I/O, C++20 DSP and Qt desktop.

## Capability maturity

| Status | Required evidence | Permitted claim |
|---|---|---|
| `active` | Existing implementation/control, valid evidence paths, regression verification and an executable acceptance gate | The scoped control exists and its stated checks run |
| `extend` | Existing partial control, explicit gap and a next measurable acceptance gate | The target is only partially delivered |
| `planned` | Explicit blocker, owner/dependency and exit criteria | Not implemented; no shipped-capability claim is permitted |
| `not_applicable` | Explicit product-scope rationale, owner and a trigger that would change the product boundary | The capability is outside this product's current scope; it is not an unfinished feature |

A `not_applicable` status is valid only when the machine-readable roadmap includes a specific reason and the validator tests that reason. Use it for capability areas outside the approved product boundary; do not use it to hide incomplete implementation.

A Markdown checkbox, JSON schema, interface declaration or successful documentation validator is not runtime implementation evidence. The DSP implementation remains offline and buffer-based; no real-time device, system audio or browser playback capability is implied.

## Priority and change control

- **P0:** data integrity, numeric safety, reproducibility, compatibility break or release blocker. Maintainer review is required.
- **P1:** core workflow correctness, audio quality, performance or measurable capability gaps. Peer/maintainer review is required.
- **P2:** bounded improvements with a supported workaround. Normal review and explicit acceptance evidence remain required.

Each behavior-changing proposal must state affected user workflows/platforms, reproduction evidence, acceptance criteria, risks/dependencies, verification commands and a rollback path. Semantic roadmap status changes are versioned pull-request changes. A generated DSP output is not replayed automatically after an ambiguous failure.

## CI execution budget

Global ceiling: **5 runner jobs**, **45 minutes per job**, **3 parallel matrix workers**.

| Workflow | Runner jobs | Per-job ceiling | Matrix parallelism |
|---|---:|---:|---:|
| VentuSample CI | 4 | 40 min | 3 |
| Quality, Repeatability and Native Contract | 1 | 15 min | 1 |
| Automation Roadmap Contract | 1 | 10 min | 1 |
| Quality remediation matrix | 1 | 15 min | 1 |
| Verified Mainline Auto-Merge | 1 | 10 min | 1 |
| Update Behind Pull Request Branches | 1 | 15 min | 1 |
| Improvement and conflict contract | 1 | 10 min | 1 |
| Safe CI Recovery | 1 | 5 min | 1 |

All scoped workflows require workflow-level concurrency and explicit cancellation behavior. Every runner job must declare `timeout-minutes`; matrices require `max-parallel`. The Python 3.10/3.11/3.12 matrix, Native C++20 build, Linux Qt and Windows Qt desktop build remain required; budgets are not an excuse to delete platform coverage.

## Safe recovery and release boundaries

Safe CI Recovery is manual-only, starts from protected `main`, permits only same-repository allow-listed validation runs with failed/timed-out conclusions, reruns failed jobs only, and allows one retry maximum. It rejects fork runs, successful/cancelled/action-required runs and exhausted attempts.

This CI workflow does not rerun audio rendering, mutate sample files, access audio devices, install drivers or flash firmware. The resilience policy keeps the runtime retry budget at zero until an operation has proven idempotency and atomic publication.

VentuSample is an offline desktop workstation, not a hosted service; production deployment is inapplicable. A gated Windows x64 release-candidate workflow now runs only by explicit dispatch from a v-prefixed tag, builds and tests the desktop application, writes a deterministic per-file SHA-256 manifest, verifies the archive checksum and creates a draft GitHub Release. Workstream 20 is `extend`, not complete: repository administrators must configure required reviewers on the `release-review` environment and a maintainer must verify the approval gate. SHA-256 is an integrity checksum, not a publisher signature. Linux distribution, signing identity, SBOM and automatic update service remain unimplemented.

## Toolchain optimization

Workstream 06 remains `extend` until a repeatable before/after baseline exists for clean and incremental CMake builds, C++/C11 compiler versions and flags, Python validation time, peak memory and artifact hashes. No default compiler, linker, Qt or packaging changes are treated as optimization without comparable measurements and full CTest/desktop regression evidence.

## Verification

```sh
python tools/validate_delivery_contracts.py --self-test
python -m unittest discover -s tests -p "test_delivery_contracts.py" -v
python scripts/validate_automation_roadmap.py --strict
python -m unittest discover -s tests -p "test_safe_rerun.py" -v
python scripts/validate_resilience_policy.py
```

Contract checks validate policies and denial paths. Native CTest, Linux DSP/Qt, Windows Qt, Python 3.10–3.12 and security workflows remain the authoritative implementation gates.
