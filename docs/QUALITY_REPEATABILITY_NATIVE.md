# Quality, Repeatability and Native Integration

## Quality
Generated samples must satisfy explicit format, duration, amplitude and metadata invariants. Validation should report actionable causes, preserve source parameters and reject malformed output without silently rewriting user intent.

## Repeatability
Generation should be deterministic when the seed and parameters are fixed. Batch work must have bounded size, stable ordering, per-item outcomes and cancellation behavior. Retrying a failed export must not silently duplicate or overwrite unrelated material. Every generated asset should retain the parameters required to reproduce it.

## Native integration
Keep the C++20 generator and DSP core authoritative. Qt 6 is the desktop integration layer, not the owner of audio-generation rules. Treat sample-pack schemas, key/scale/BPM metadata and DAW-facing exports as versioned boundaries. Preserve platform-native file handling and graceful desktop build behavior when optional Qt components are unavailable.

## VentuSample application
Protect the native generator, sample metadata, deterministic output contract and pack fixture validation. Multi-sample batch generation and resumable export should remain explicitly planned until their limits, cancellation and partial-failure behavior are implemented and tested.

## Acceptance and evidence
quality-repeatability-native.v1.json records implementation and verification evidence for every criterion. The validator checks manifest structure, unique IDs and that evidence files exist; it does not substitute for compiled audio tests, cross-platform desktop builds, listening checks or release verification.

Use implemented only when code and a maintained test or workflow support the requirement. Use planned for incomplete work. Use not_applicable only with a written reason.

Local checks: python tools/validate_quality_repeatability_native.py --self-test and python tools/validate_quality_repeatability_native.py --json.
