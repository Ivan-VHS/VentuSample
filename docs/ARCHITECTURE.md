# Architecture and audio contract

**Baseline:** C++20 offline renderer · Qt 6 Widgets · PCM16/PCM24/Float32 WAV · versioned JSON manifests.

| Layer | Responsibility |
|---|---|
| Qt desktop | controls, folders, results/log, preferences |
| Parameters | typed settings, validation, seed, limits |
| Renderer | pattern scheduler, synthesis, stereo float buffers |
| Asset I/O | PCM16 import, 4-slice chop, pitch resample, WAV export |
| Master | sampler color, linked stereo compression, sample-peak ceiling |
| Manifest | settings snapshot, key/mode, peak/RMS/crest |
| Plugin host | versioned built-in registry, enable/disable lifecycle, bounded handlers and public registration API |
| Audio Suite | 66-ID effect catalogue, deterministic stereo float DSP, ordered offline chains and bounded PCM16 WAV I/O |
| CINEFORGE | Seeded cinematic sample families, Dry/Wet, Attack/Body/Tail, 24-bit PCM / Float32 WAV, preview and pack manifest |
| Stochastic router | Seeded Micro-Variation, Round-Robin, Chaos One-Shot, locked slots, 2–4 offline parallel branches, RMS/DC/peak safety |
| Tests | validation, repeatability, variations, WAV round-trip, schemas, plugin lifecycle and all 66 effect processors |

## Audio effect processing

The Audio Suite uses an independent AudioSuiteBuffer contract (stereo float, 8–384 kHz, at most 20 million frames) and a versioned 66-effect catalogue. Parameters are normalized to 0..1 and validated before execution. An effect chain is rendered on a working copy and only committed to the caller buffer when every effect succeeds. WAV import accepts PCM16 mono/stereo up to 128 MiB; mono is expanded to dual-mono. Export writes a new stereo PCM16 RIFF/WAVE file. CINEFORGE separately writes stereo PCM24 or IEEE Float32 RIFF/WAVE at 48/96 kHz.

The current implementation is offline and scalar. The DSP kernels are deterministic effect models and do not require a GPU, external library, audio device, or network access. Live audio callbacks, VST3/AU hosting, neural inference and full physical room reconstruction remain out of scope.

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


## CINEFORGE and stochastic routing

CINEFORGE creates 8–12 seeded scene-labelled procedural assets. Each asset exports Dry, Wet and synthesis-time Attack/Body/Tail layers. The prompt is stored as metadata; the generator currently uses explicit procedural sound-family models rather than neural text-to-audio. cineforge.pack.v1 records the render settings and output paths.

The stochastic router builds deterministic chains using an explicit seed. Micro-Variation preserves locked slots, Round-Robin precomputes 4–8 chains and Chaos One-Shot builds one new chain. Optional 2–4-way parallel branches are rendered offline from a shared source buffer. An output safety pass can match RMS within a bounded gain range, remove DC and enforce a sample-peak ceiling. It is not a real-time host: PDC, real-time feedback, live crossfades and a true-peak limiter are not claimed.

The product concept describes 199 processor slots, but only the current 66 Audio Suite processors are implemented as DSP. The additional 133 quantum/quark-themed concepts are not registered as available processors until implemented and tested.
