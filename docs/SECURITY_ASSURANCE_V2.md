# Repository Security Assurance v2

## Coverage

The federated assurance workflow audits tracked source and configuration files of **Pytyliki, Reaper, OPlague and VentuSample**. It does not execute inspected source during static scanning. Findings contain rule ID, severity, repository-relative path and line number only; secret values and source snippets are never included.

### Automated controls

- **Source audit:** high-confidence private-key/token patterns, hard-coded credential assignments, developer-machine paths, sensitive-value logging and selected risky APIs.
- **CodeQL:** C/C++ and GitHub Actions source analysis with the extended query suite.
- **Dependency vulnerabilities:** OSV-Scanner compares dependency advisories on Pull Requests and performs a scheduled full scan; GitHub Dependency Review gates pull-request dependency changes, while Dependabot for GitHub Actions and an offline inventory of CMake/package declarations, Python manifests, lockfiles and exact pins.
- **Fuzz smoke:** the real ventu::loadWave parser receives bounded malformed and RIFF-shaped byte streams; the real processQuantumAudio routine receives 128 deterministic valid control/sample cases and output checks.
- **Privacy regression:** synthetic credential detection, placeholder behavior, no-secret-output assertions and privacy-policy flags are tested.
- **Federation:** all four repositories are scanned for source/dependency findings, and common policy/tool checksums are compared.

## Run locally

1. Run scripts/security_audit.py and scripts/dependency_audit.py.
2. Run scripts/test_privacy_regressions.py.
3. Configure CMake with VENTU_BUILD_DESKTOP=OFF and VENTU_BUILD_TESTS=ON.
4. Build and run scripts/fuzz_smoke.py.

The fuzz wrapper reruns the ventusample-quantum-audio CTest target. The normal CMake test suite and sanitizer build remain additional controls.

## Shared score

Every repository uses the same 100-point weights:

| Category | Weight |
|---|---:|
| Secret hygiene | 20 |
| Privacy leak prevention | 15 |
| Source-risk review | 15 |
| Fuzz robustness | 15 |
| Dependency pin hygiene | 5 |
| Known dependency vulnerabilities (OSV) | 10 |
| Privacy regressions | 10 |
| Federation consistency | 10 |

The federated report additionally calculates a combined score from policy/tool parity, aggregate source findings and dependency pin hygiene. The gate fails on critical credential findings, known dependency vulnerabilities, failed fuzz/privacy/federation checks or a score below 70.

## Boundaries

The static scanner is heuristic and may produce false positives or miss novel issues. CodeQL supplements it with language-aware analysis. Dependency inventory/pinning is offline; GitHub Dependency Review checks supported PR changes but does not certify all external system packages. The deterministic fuzz corpus is smoke fuzzing, not continuous coverage-guided fuzzing. The offline-first design and metadata-only plugin discovery remain unchanged; this workflow does not introduce audio upload or arbitrary plugin execution.
