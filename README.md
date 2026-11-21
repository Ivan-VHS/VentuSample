# VentuSample

C++20 · Qt 6 Widgets · Windows/Linux where Qt is available · Offline · Apache-2.0
**Разработчик:** [Ivan-VHS](https://github.com/Ivan-VHS)

Native workstation for deterministic procedural hip-hop sample packs. The scalar renderer exports PCM16 stereo WAV and versioned JSON manifests; it does not require a DAW or audio device.

## Internal plugin suite

The 19 shared internal products are built-in plugins in the native PluginManager. Open **Плагины → Менеджер плагинов** to inspect, enable/disable and run tools, or discover local extension manifests. The C++20 SDK provides reviewed handler registration, lazy initialization, shutdown and diagnostics. Manifest discovery is metadata-only; arbitrary DLL/SO loading remains disabled. See [Plugin platform and SDK](docs/PLUGINS.md) and the [shared manifest schema](schemas/ivan-vhs.plugin.v1.schema.json).

## Quantum Audio Engine — 133 DSP modules

A native C++ audio lab is available from **Плагины → Quantum Audio Engine — 133 DSP**. It loads PCM16 WAV, builds an ordered processing chain from five families, processes float audio offline, displays signal measurements and exports stereo WAV. The 133 module identities use 21 reusable, tested DSP kernels. This is a local offline processor, not a live DAW or third-party plugin loader. See [Quantum Audio Engine](docs/QUANTUM_AUDIO_ENGINE.md) for the complete catalogue and audio contract.

## Atomic-Quantum 133 simulation layer

A separate C++20 state-vector research library provides 133 canonical operation IDs plus bounded state preparation, Hadamard/phase/Pauli/CNOT/SWAP gates, Bell/GHZ states, seeded projective measurement, simple stochastic noise trajectories and fidelity checks. It is distinct from the 133-module audio DSP engine above; it does not claim access to physical quantum hardware. See [Atomic-Quantum 133](docs/ATOMIC_QUANTUM_133.md).

## Stochastic routing

The optional **Stochastic Router** processes rendered stereo buffers before WAV export. Its weighted pool contains low/high-pass filters, saturator, bitcrusher, delay, reverb, ring modulation and stereo width. Choose active effects, hybrid/serial/parallel topology, file/bar/scheduled-onset triggers and intensity. Fixed seed + settings produce repeatable routes; manifest v2 records the selected route names per file. Wet is bounded to 65%, feedback to 72%, transitions fade over 5–20 ms, RMS gain compensation is bounded, and output is clamped to a −0.3 dBFS sample-peak ceiling. This is offline DSP, not a real-time DAW host.

## Generator matrix

| Output | Content |
|---|---|
| Full Loop | drums, melody, bass, optional source chops |
| Melody | chords and seeded note variations |
| Drums | kick, snare, hats, percussion, rolls |
| Bass | generated bass patterns and optional glide |
| One-Shot Kit | 8 hits; tonal labels only for pitched sounds |

14 procedural profiles: Boom Bap, Billionaire Golden Era, East Coast, Harlem Dip-Trap, G-Unit, Modern Trap, Atlanta Trap, UK Drill, UK Grime, Memphis, Jazz Rap, G-Funk, Boom Trap, Cripple Trill.

| Control | Range |
|---|---|
| BPM / bars / variations | 60–190 / 1–16 / 1–32 |
| Key / scale | C–B / 6 families |
| Swing | 50–68% |
| Density, grit, saturation, stereo | 0–100% |
| High-pass / low-pass | 20–300 Hz / 2–20 kHz |
| Snare offset | −30…+30 ms |
| Bus compression | 1:1–12:1 |
| Sample rate | 22.05 / 44.1 / 48 / 96 kHz |
| Source WAV | PCM16 mono/stereo, 8–192 kHz, ≤128 MiB, ≤20M frames |
| Chop pitch / mix | −12…+12 semitones / 0–100% |

## Build and test

CMake 3.21+, C++20 compiler; Qt 6 Widgets for desktop.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=ON -DVENTU_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

If Qt is unavailable, portable DSP core and tests can still build; the desktop target is skipped.

## Export and limits

- Fixed seed/settings produce repeatable output; rendering runs on a worker thread. Stochastic routes use the same seed/variation model.
- Manifest v2 stores effective settings, `musical_key`, `mode_family`, `scale` and per-file peak/RMS/crest. v1 remains available for older packs.
- Full Loop/Melody/Bass receive key/mode labels; unpitched drums do not. Imported WAV is not automatically key-detected.
- Master ceiling is approximately −0.3 dBFS sample peak, not an oversampled true-peak limiter.
- Semitone resampling changes pitch and playback speed together; it is not time-stretch.
- No network telemetry, background audio capture, real-time callback or implicit native-library loading.
- Import/export requires appropriate source rights; a hash proves byte identity, not ownership.

## Live desktop capture

The Windows CI workflow captures the actual 1920×1080 Qt app and uploads `ventusample-live-windows-screenshot`. Open the [CI workflow](https://github.com/Ivan-VHS/VentuSample/actions/workflows/ci.yml), choose a successful run and download the artifact. GitHub retains it for 30 days; it is not a permanent repository image.

[Features](docs/FEATURES.md) · [Quantum Audio Engine](docs/QUANTUM_AUDIO_ENGINE.md) · [Architecture](docs/ARCHITECTURE.md) · [Russian guide](docs/USER_GUIDE_RU.md) · [QMR-20](docs/QUALITY_MONOLITH_RHYTHM.md) · [Reaper audit](docs/REAPER_PACK_AUDIT.md) · [Source rights](docs/SOURCE_INTEGRATION.md) · [Docs index](docs/README.md)

[Apache-2.0](LICENSE)
