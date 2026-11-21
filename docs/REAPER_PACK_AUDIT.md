# Reaper pack audit — VentuSample integration

VentuSample owns audio generation and export. Reaper is an optional downstream consumer for validating and comparing the JSON manifests emitted by VentuSample. The two applications remain separate; no background service, network connection or shared runtime is required.

## Export contract

VentuSample exports a folder containing PCM16 WAV assets and a ventusample_manifest.json document that conforms to schemas/ventusample.pack.v1.schema.json.

The manifest records:

- Generator/schema identity and WAV format.
- Effective style, target, tempo, key, scale, bars and variation count.
- Swing, density, grit, saturation, stereo, filters, snare offset and bus compression.
- Triplet, humanization and vinyl-dust switches.
- Seed, sample rate, sampler character, timbre and optional chop-source settings.
- Per-file relative filename, peak, RMS, crest factor and variation index.

It records a source filename, not an absolute host path. WAV assets remain local to the export folder.

## Reaper consumer

Reaper exposes two built-ins for a .rpr workflow:

- ventusample_metrics(pack) validates the v1 manifest and returns 22 normalized finite descriptors. The descriptor set includes generation parameters and aggregate file measurements.
- ventusample_compare(reference, candidate[, library_path]) compares two validated manifests and returns a read-only reaper.ventusample-comparison.v1 report containing metadata flags, per-component deltas, Euclidean descriptor distance, the selected backend and deterministic report SHA-256.

Reaper's Python metric engine is used by default. A caller may supply an explicit absolute path to a trusted Pytyliki ABI v1 library to route the distance calculation through the existing native numerical interface. This is an opt-in metric cross-check, not a Pytyliki audio renderer or a direct VentuSample-to-Pytyliki DSP bridge.

## How to use it

1. Generate two packs in VentuSample and export them into separate folders.
2. Make the two ventusample_manifest.json files accessible relative to the Reaper audit script.
3. In Reaper, run examples/ventusample_pack_audit.rpr after editing the two manifest paths.
4. Review the produced ventusample-comparison-report.json.

The native option is selected explicitly in the comparison call:

~~~reaper
make report := ventusample_compare(
    reference,
    candidate,
    "C:/trusted/Pytyliki/build/Release/pytyliki_ffi.dll"
)
~~~

## What the audit does not do

The consumer reads the JSON manifest only. It does not open, decode, copy, upload, overwrite or delete WAV files. It does not compare waveform samples, perform listening tests, assess musical quality or decide which style is better. The Euclidean distance describes a difference between normalized settings and generator-supplied measurements; it is not a quality score.

Reaper does not send render requests to VentuSample in this release. The reverse render-request edge and direct VentuSample ↔ Pytyliki ABI edge remain planned. OPlague's device/driver functionality is deliberately not coupled to the audio workflow.

## Related contracts

- [VentuSample pack JSON Schema](../schemas/ventusample.pack.v1.schema.json)
- [Four-project OCTA-8 topology](../federation/octa-pyramid.v1.json)
- [Reaper federation implementation](https://github.com/Ivan-VHS/Reaper/blob/main/reaper/ventusample.py)
- [Reaper example](https://github.com/Ivan-VHS/Reaper/blob/main/examples/ventusample_pack_audit.rpr)
- [Shared four-project matrix](https://github.com/Ivan-VHS/Reaper/blob/main/docs/PROJECT_FEDERATION_MATRIX.md)
