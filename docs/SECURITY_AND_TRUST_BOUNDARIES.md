# Security and trust boundaries — VentuSample

## Scope and current operating model

VentuSample is a C++20/Qt workstation for deterministic procedural audio generation, offline DSP and PCM16 WAV import/export. Plugin manifest discovery is metadata-only. A discovered manifest is not executable until a reviewed native handler is explicitly attached. The in-process plugin host is not an OS security sandbox.

## Assets and trust boundaries

| Boundary | Untrusted material | Required handling |
|---|---|---|
| WAV import | RIFF chunks, sizes, channels, sample rate and sample bytes | validate chunk boundaries, frame counts, format and configured size limits |
| JSON manifests | external fields, versions and paths | schema/version validation; reject unknown unsafe path semantics |
| Plugin discovery | local plugin manifests and entry metadata | metadata-only parsing; keep entries disabled until reviewed handler registration |
| Native handlers | C++ code and callback behavior | explicit trusted registration; bounded lifecycle and diagnostics |
| DSP pipeline | audio buffers and parameter values | finite-value checks, bounded buffers, validate parameters and deterministic error handling |
| Render workers | task count, duration and cancellation | bounded work queues, cancellation and no unbounded retries |
| Export | generated samples and manifest fields | write to controlled destinations; publish only complete outputs |

## Non-negotiable safety rules

- Audio files, manifests and external assets are untrusted input.
- Validate file size, RIFF chunk arithmetic, frame count, channel count and sample format before allocating or reading.
- Do not execute code from a plugin manifest or automatically load arbitrary DLL/SO files.
- In-process native handlers have host-process privileges; do not claim they are sandboxed.
- Keep DSP offline unless a separately reviewed real-time design is implemented.
- Respect source rights. A content hash proves byte identity, not ownership or licensing.

## Failure behavior

Malformed or oversized WAV files must be rejected before out-of-bounds reads or unbounded allocations. Non-finite parameters and samples must be handled deterministically. A failed render must not publish a partial file as a successful artifact. Plugin initialization or callback failure must leave the plugin disabled or stopped and must not corrupt the render queue.

## Verification evidence

For changes to import, plugin or DSP boundaries, add:
1. WAV parser tests for truncated chunks, overflow, invalid formats and size ceilings;
2. manifest schema tests and path traversal cases;
3. plugin lifecycle tests for registration, disable, failure and shutdown;
4. DSP tests for finite values, clipping, boundary parameters and deterministic seeds;
5. render cancellation and resource-limit tests;
6. fuzz targets for WAV/JSON parsers where supported.

Run the AppGuard catalog checks:

```sh
python scripts/validate_beelzebul_appguard.py
python -m unittest discover -s tests -p "test_beelzebul_appguard.py" -v
```

These commands validate catalog structure and metadata. They do not prove arbitrary plugin isolation, real-time safety or all 666 controls.

## AppGuard status vocabulary

- **planned** — catalogued; no executable enforcement claim.
- **partial** — a named subset is implemented and evidenced.
- **implemented** — enforcement code and focused tests exist.
- **verified** — implementation, tests and review evidence are linked.

The current AppGuard PR adds the control catalog, schema, validator, regression tests and CI workflow. It does not activate 666 runtime protections.

## Change and release checklist

- Keep WAV, manifest and plugin contracts versioned and linked from the docs index.
- Update documented numeric limits whenever parser or renderer limits change.
- Distinguish offline processing from real-time audio behavior.
- Keep screenshots genuine and do not imply unavailable plugin isolation.
- Document sample/source rights and avoid bundling assets without a clear license.
