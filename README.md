# VentuSample

C++20 · Qt 6 Widgets · Windows/Linux where Qt is available · Offline · Apache-2.0

Native workstation for deterministic procedural hip-hop sample packs. The scalar renderer exports PCM16 stereo WAV and versioned JSON manifests; it does not require a DAW or audio device.

## Internal plugin suite

The 18 shared internal products are built-in plugins in the native PluginManager. Open the **Плагины** menu to inspect capabilities, enable/disable modules and run read-only tools. See [Plugin host and SDK](docs/PLUGINS.md). The C++ SDK supports registering reviewed in-process handlers; arbitrary DLL/SO loading is intentionally not enabled.

## Ventu Audio Suite — 66 DSP effects

The native **Ventu Audio Suite** adds 66 separately addressable stereo processors in six families: space/acoustics, granular time, timbre/synthesis, dynamics/punch, frequency/phase, and generative/rhythm. Open **Плагины → Ventu Audio Suite — 66 DSP эффектов** to import a PCM16 WAV, assemble an ordered effect rack, edit per-effect Amount/Wet/Rate/Tone/Width controls, and export a new WAV.

The effects are versioned built-in modules using the portable C++20 offline DSP API in src/audio_suite.hpp. They are processed left-to-right on a worker thread; the source file is never overwritten. The DSP models are deterministic algorithmic approximations, not neural inference or full physical room simulation. This release does not claim VST3/AU hosting, live input monitoring, or real-time playback.

See [Audio Suite guide](docs/AUDIO_SUITE.md) for the full 66-effect catalogue, signal flow, supported WAV contract, and limitations.

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

- Fixed seed/settings produce repeatable output; rendering runs on a worker thread.
- Manifest v2 stores effective settings, `musical_key`, `mode_family`, `scale` and per-file peak/RMS/crest. v1 remains available for older packs.
- Full Loop/Melody/Bass receive key/mode labels; unpitched drums do not. Imported WAV is not automatically key-detected.
- Master ceiling is approximately −0.3 dBFS sample peak, not an oversampled true-peak limiter.
- Semitone resampling changes pitch and playback speed together; it is not time-stretch.
- No network telemetry, background audio capture, real-time callback or implicit native-library loading.
- Import/export requires appropriate source rights; a hash proves byte identity, not ownership.

## Live desktop capture

The Windows CI workflow captures the actual 1920×1080 Qt app and uploads `ventusample-live-windows-screenshot`. Open the [CI workflow](https://github.com/Ivan-VHS/VentuSample/actions/workflows/ci.yml), choose a successful run and download the artifact. GitHub retains it for 30 days; it is not a permanent repository image.

[Features](docs/FEATURES.md) · [Architecture](docs/ARCHITECTURE.md) · [Russian guide](docs/USER_GUIDE_RU.md) · [QMR-20](docs/QUALITY_MONOLITH_RHYTHM.md) · [Reaper audit](docs/REAPER_PACK_AUDIT.md) · [Source rights](docs/SOURCE_INTEGRATION.md) · [Docs index](docs/README.md)

[Apache-2.0](LICENSE)
