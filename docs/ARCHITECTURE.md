# Architecture and audio contract

**Baseline:** C++20 scalar offline renderer · Qt 6 Widgets · PCM16 WAV · manifest v2.

| Layer | Responsibility |
|---|---|
| Qt desktop | controls, folders, results/log, preferences |
| Parameters | typed settings, validation, seed, limits |
| Renderer | pattern scheduler, synthesis, stereo float buffers |
| Asset I/O | PCM16 import, 4-slice chop, pitch resample, WAV export |
| Master | sampler color, linked stereo compression, sample-peak ceiling |
| Manifest | settings snapshot, key/mode, peak/RMS/crest |
| Plugin host | versioned built-in registry, enable/disable lifecycle, bounded handlers and public registration API |
| Quantum Audio Engine | 133 typed DSP module identities, 21 reusable processing kernels, ordered offline chain and WAV I/O |
| Tests | validation, repeatability, variations, WAV round-trip, schemas, plugin lifecycle and 133 DSP module checks |

## Numeric limits

| Parameter | Range |
|---|---|
| BPM / bars / variations | 60–190 / 1–16 / 1–32 |
| Swing | 50–68% |
| Density / grit / saturation / stereo | 0–100% |
| High-pass / low-pass | 20–300 Hz / 2–20 kHz |
| Snare offset | −30…+30 ms |
| Bus compressor | 1:1–12:1 |
| Sample rate | 22050 / 44100 / 48000 / 96000 Hz |
| Source WAV | PCM16 mono/stereo, 8–192 kHz |
| Source bounds | 128 MiB / 20,000,000 frames |
| Pitch / mix | −12…+12 semitones / 0–100% |

Low-pass must exceed high-pass and stay below 0.49× sample rate. Source transposition uses resampling, not time-stretch.

## Manifest and tonality

| Asset | Key metadata |
|---|---|
| Full Loop / Melody / Bass | selected root + scale |
| Drums | none |
| One-Shot Kit | pitched bass/pluck only |

v2 records `musical_key`, `mode_family` and `scale`; v1 remains supported. Labels describe generation settings, not auto-analysis of imported audio. Existing output files are not overwritten.

Golden Era profile: fixed source-slice order 0–2–1–3; kick/snare-weighted voicing and restrained hats. User BPM, swing and snare offset remain authoritative.

Rendering is offline on a worker thread. No real-time callback or audio-device access is provided.
