# CINEFORGE — Hollywood Stem & Sample Engine

CINEFORGE is the native, offline cinematic sound-family generator inside VentuSample. Open it from the Плагины menu: AETHER-ANAS // CINEFORGE Hollywood Stem Engine.

## Workflow

1. Enter a scene description (stored in the pack manifest), BPM, minor key, bars, pack size (8–12), seed, sample rate and encoding.
2. Choose a local output directory.
3. Generate a deterministic pack. A new folder is created; existing folders are never overwritten.
4. Audition CINEFORGE_Preview.wav and copy the generated WAV files into a DAW.

## Asset family

The current procedural family catalogue includes SubBoom, TitanImpact, BrassPulse, MetalHit, Ostinato, PercussionLoop, Riser, Atmosphere, HybridAccent and ImpactTail. Pack size selects the first N families in a stable order.

Each object exports five independent stereo WAV files:
- Dry — dry composite.
- Wet — deterministic multi-tap reflection variant.
- Attack — transient/noise and initial impulse layer.
- Body — tonal harmonic body.
- Tail — decaying noise and harmonic atmosphere.

The stems are generated separately by the synthesis engine. They are not estimated from an arbitrary imported mix and do not claim neural source separation.

## File formats

| Control | Supported values |
|---|---|
| Sample rate | 48,000 or 96,000 Hz |
| Encoding | PCM 24-bit or IEEE Float 32-bit |
| Channels | Stereo |
| Pack size | 8–12 assets |
| BPM | 60–190 |
| Bars | 1–16 |
| Seed | 64-bit engine field; desktop exposes signed 31-bit seed |
| Prompt | 1–512 UTF-8 bytes, stored as metadata |

File names include CF_<BPM>bpm_<Key>_<Family>_<Index>_<Layer>.wav. The manifest uses schema cineforge.pack.v1 and records the seed, scene, timing, key, encoding and exported file names.

## Determinism and safety

A fixed seed and settings produce byte-identical output within the same supported implementation. Existing pack directories are not overwritten. Rendering is offline in a worker thread. No network model, telemetry, audio-device capture or external plugin is used.

The scene prompt is descriptive metadata in this MVP; it does not directly drive a neural text-to-audio model. Wet processing is deterministic multi-tap reflection, not a physically reconstructed room. Attack, Body and Tail are synthesis layers, not a machine-learning separation result.

## Tests

The CINEFORGE test target verifies pack validation, eight-asset generation, five exports per asset, manifest/preview creation, PCM24 and Float32 WAV headers, deterministic bytes for a fixed seed, and refusal to overwrite an existing pack folder.
