# Beelzebul — Multi-Layer Repository Guard

**Version:** 1.0.0 · **Codex:** 66 rules · **Mode:** read-only security audit

Beelzebul is installed as the same small, standard-library Python security tool in **Pytyliki, Reaper, OPlague and VentuSample**. It audits repository files without importing, compiling, executing, rewriting or uploading inspected source code.

## The 66-rule model

| Layer | Coverage | Purpose |
|---|---:|---|
| Atomic bytes | A01–A15 (15) | Credentials, key material, Unicode controls, file integrity and source-byte anomalies |
| Atomic modules | M01–M15 (15) | Dynamic execution, process boundaries, deserialization, TLS, SQL, paths and resource hazards |
| Nuclear repository | N01–N15 (15) | GitHub Actions permissions, untrusted events, action pinning, container and dependency controls |
| Quantum correlations | Q01–Q15 (15) | Cross-file and within-file combinations that individual checks can miss |
| Common control | G01–G06 (6) | Canonical policy, severity, deterministic evidence, expiring exceptions and unified scoring |

“Quantum” describes correlation analysis across files, workflows and trust boundaries. It does not imply quantum hardware or probabilistic execution.

## Run locally

    python security/beelzebul/scanner.py --root . --format text
    python security/beelzebul/scanner.py --root . --format json --output beelzebul-report.json
    python -m unittest discover -s security/beelzebul -p "test_*.py" -v
    python security/beelzebul/federation.py --root . --ref main

For a pull-request range:

    python security/beelzebul/scanner.py --root . --base-ref <BASE_SHA> --changed-only --fail-on high

The scanner checks the current tree and added lines in the Git history range. Reports include rule IDs, severity, locations and remediation-oriented messages; matched credential contents are never copied into reports.

## CI and inter-repository operation

The .github/workflows/beelzebul.yml workflow runs regression, privacy and fuzz tests and blocks new critical/high findings in changed files. Scheduled/manual full audits emit machine-readable reports as workflow artifacts. The federation auditor compares the canonical policy, exception register, scanner, tests, documentation and workflow across the four public repositories. Strict peer parity runs on a schedule, allowing a staged multi-repository rollout without blocking intermediate pull requests.

The scanner uses only Python's standard library. It is intentionally read-only and has no network access; the separate federation auditor performs bounded HTTPS GET requests to public peer files and needs no token.

## Severity and response

- **Critical:** exposed private material, unsafe privileged workflow combinations or direct compromise paths.
- **High:** credential literals, unsafe trust-boundary operations and correlated command-execution risks.
- **Medium:** dependency drift, disabled safeguards and suspicious constructs requiring review.
- **Low:** hygiene signals that can contribute to cumulative risk.

The deterministic score uses shared weights: critical 25, high 12, medium 5 and low 1, capped at 100. New critical/high findings in changed files fail CI. Full scheduled scans initially report existing findings without blocking unrelated maintenance; this separates legacy debt from newly introduced regressions.

## Exceptions

The .beelzebul/exceptions.v1.json register is empty by default. An exception must specify one exact rule_id, one exact repository-relative path, a reason and an ISO expires_at date. Wildcards, absolute paths, traversal paths and expired exceptions are rejected. Removing the underlying risk is preferred over suppression.

## Scope and limitations

This is deterministic static analysis, not a replacement for branch protection, code review, runtime sandboxing, secret-provider revocation or vendor vulnerability databases. Dependency checks identify lockfile gaps and floating declarations; they do not claim exhaustive CVE coverage. A detected credential must be revoked at its issuing provider and removed from affected Git history where appropriate. No automatic credential revocation, branch rewrite, network blocking or destructive remediation is performed.

Each repository retains its own scanner and CI entry point. The federation check detects policy/tool drift; rule IDs, thresholds, report schema and tests remain common across all peers.
