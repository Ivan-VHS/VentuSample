# Resilience and trust policy

**Policy version:** 1.0  
**Implementation status:** `contract_only`  
**Scope:** offline sample-pack generation, bounded WAV import/export, deterministic DSP and the local Qt desktop.

This policy is a machine-readable safety and recovery contract. It is validated in CI, but it does **not** claim that timeout enforcement, retry orchestration, circuit breakers, audit logging or a runtime trust broker are already implemented.

## Execution model

VentuSample remains a single-machine, offline-first application. Rendering and DSP operate on local buffers. Network access, background audio capture, remote execution, system-audio control and implicit native-library loading are disabled.

Input WAV files remain untrusted. Existing format, sample-rate, channel, file-size, frame-count and RIFF-chunk limits continue to apply. Numeric-integrity failures, invalid parameters and resource-limit violations fail closed; the application must not silently replace the selected algorithm with a lower-quality fallback.

## Recovery policy

The configured timeout is a future upper bound for cancellable work, not a claim that all current operations are interruptible. The automatic retry budget is deliberately **zero** for now: deterministic rendering and output writes are not replayed after an ambiguous failure.

A future retry may be introduced only for an operation proven idempotent, with:
- an explicit attempt ceiling and bounded exponential backoff with jitter;
- a unique staging destination and atomic publication for generated files;
- integrity checks before promotion and unchanged output on failure;
- observable attempt/failure records and the same regression gates after recovery.

Do not retry rendering or file publication merely because a process returned an error. Never automatically retry destructive actions, install drivers, modify hardware or publish releases. Persistent failures remain failures.

## Trust, migration and supply chain

The default trust decision is deny. Schemas and dependency versions must be validated; unverified upstream inputs are rejected. Any future privileged action requires an explicit human decision. Migration requires a backup, rollback plan, staged rollout and compatibility check.

Release artifacts must have a dependency lock, SBOM, provenance, reproducible-build evidence and signing. A successful policy check validates the declaration only; it does not prove the presence of those release artifacts.

## Local observability

Structured diagnostics and audit evidence are local by default. Secrets and machine-specific paths must not be emitted. Evidence is labelled as measured, derived, estimated or unknown. Manual overrides require a reason, an audit record and an expiry.

## Verification

```sh
python scripts/validate_resilience_policy.py
python -m unittest discover -s tests -p "test_resilience_policy.py" -v
python scripts/validate_automation_roadmap.py --strict
```

The validator is standard-library-only. The policy remains `contract_only` until individual runtime controls are implemented, tested and reviewed against the audio pipeline.
