# VentuSample — procedural sample workstation

[CI](https://github.com/Ivan-VHS/VentuSample/actions/workflows/ci.yml) · C++20 · Qt 6 Widgets · Windows/Linux · Offline · Apache-2.0

VentuSample is a native workstation for deterministic procedural hip-hop sample packs. Its scalar renderer exports PCM16 stereo WAV files and versioned JSON manifests without requiring a DAW, audio device or network connection.

## Live Windows desktop

<p align="center"><img src="assets/screenshots/ventusample-studio.png" alt="VentuSample running on Windows 11" width="100%"></p>
<sub>Actual 1920×1080 Windows 11 desktop capture from the Qt CI build. The screenshot shows the generator workbench; plugin discovery and the DSP laboratory are separate menu actions.</sub>

## Generator matrix

| Output | Content |
|---|---|
| Full Loop | drums, melody, bass and optional source chops |
| Melody | chords and seeded note variations |
| Drums | kick, snare, hats, percussion and rolls |
| Bass | generated bass patterns and optional glide |
| One-Shot Kit | 8 hits; tonal labels only for pitched sounds |

There are 14 procedural profiles: Boom Bap, Billionaire Golden Era, East Coast, Harlem Dip-Trap, G-Unit, Modern Trap, Atlanta Trap, UK Drill, UK Grime, Memphis, Jazz Rap, G-Funk, Boom Trap and Cripple Trill.

| Control | Supported range |
|---|---|
| BPM / bars / variations | 60–190 / 1–16 / 1–32 |
| Key / scale | C–B / 6 scale families |
| Swing | 50–68% |
| Density, grit, saturation, stereo | 0–100% |
| High-pass / low-pass | 20–300 Hz / 2–20 kHz |
| Snare offset | −30…+30 ms |
| Bus compression | 1:1–12:1 |
| Sample rate | 22.05 / 44.1 / 48 / 96 kHz |
| Source WAV | PCM16 mono/stereo, 8–192 kHz, ≤128 MiB, ≤20M frames |
| Chop pitch / mix | −12…+12 semitones / 0–100% |

## Internal plugin platform

The native PluginManager includes 18 built-in internal tools. The **Плагины → Менеджер плагинов** view supports inspection, enable/disable and bounded dispatch. A public C++20 SDK exposes reviewed handler registration, lazy initialization, shutdown callbacks and diagnostics.

Local `plugin.json` manifests can be discovered recursively, but discovery is metadata-only. A discovered entry remains disabled until a reviewed native handler is explicitly attached. Automatic DLL/SO loading, manifest-driven code execution, marketplace installation and sandbox claims are not supported. The public manifest contract is [ivan-vhs.plugin.v1](schemas/ivan-vhs.plugin.v1.schema.json). See [Plugin platform and SDK](docs/PLUGINS.md).

## Quantum Audio Engine — 133 DSP modules

Open **Плагины → Quantum Audio Engine — 133 DSP** for the separate offline audio laboratory. It contains 133 stable module identities across five families and routes them through 21 reusable, tested C++ DSP kernels. Load a supported WAV, assemble an ordered chain, adjust Amount / Tone / Rate / Wet-Dry, process offline and export stereo PCM16 WAV.

The catalogue is a creative signal-processing system, not physical quantum computation. It is not a real-time audio engine, DAW, VST/AU host or arbitrary native-library loader. See [Quantum Audio Engine](docs/QUANTUM_AUDIO_ENGINE.md) for input/output constraints, catalogue and DSP boundaries.

## Build and test

Requirements: CMake 3.21+, a C++20 compiler and Qt 6 Widgets for the desktop application.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=ON -DVENTU_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

Without Qt, the portable DSP core and tests can still build; the desktop target is skipped. CI checks Python project contracts, native C++20 code and the Qt desktop build on Linux and Windows.

## Export, determinism and limits

- Fixed seed and settings produce repeatable renders; rendering runs on a worker thread.
- Manifest v2 records effective settings, `musical_key`, `mode_family`, `scale` and per-file peak/RMS/crest. Manifest v1 remains readable for older packs.
- Full Loop, Melody and Bass may receive key/mode labels. Unpitched drums do not. Imported WAV files are not automatically key-detected.
- Master ceiling is approximately −0.3 dBFS sample peak; it is not an oversampled true-peak limiter.
- Semitone resampling changes pitch and playback speed together; it is not time-stretching.
- No network telemetry, background audio capture, real-time callback or implicit native-library loading.
- Import/export requires appropriate source rights. A hash proves byte identity, not ownership.

[Features](docs/FEATURES.md) · [Plugin SDK](docs/PLUGINS.md) · [Quantum Audio Engine](docs/QUANTUM_AUDIO_ENGINE.md) · [Architecture](docs/ARCHITECTURE.md) · [Russian guide](docs/USER_GUIDE_RU.md) · [QMR-20](docs/QUALITY_MONOLITH_RHYTHM.md) · [Reaper pack audit](docs/REAPER_PACK_AUDIT.md) · [Source rights](docs/SOURCE_INTEGRATION.md) · [Anatidae design](docs/ANATIDAE_DESKTOP_DESIGN_SYSTEM.md) · [Docs index](docs/README.md)

[Apache-2.0](LICENSE)
