# 🎛️ VentuSample — Procedural Audio Workstation

<p align="center">[English](README.md) · [Українська](README.uk.md) · [Русский](README.ru.md) · [简体中文](README.zh-CN.md)</p>

> **Psychoporter 666** — architectural precision × exploratory energy × traceable evidence. [Shared documentation style](https://github.com/Ivan-VHS/DAK/blob/main/docs/PSYCHOPORTER_DOCUMENTATION_STYLE_V1.md).  
> **Editorial revision · 2026-11-28:** presentation and navigation updated; original authorship, license terms and third-party attribution are retained.


[![CI](https://img.shields.io/github/actions/workflow/status/Ivan-VHS/VentuSample/ci.yml?branch=main&label=CI)](https://github.com/Ivan-VHS/VentuSample/actions/workflows/ci.yml) [![Release](https://img.shields.io/github/actions/workflow/status/Ivan-VHS/VentuSample/release.yml?branch=main&label=Release)](https://github.com/Ivan-VHS/VentuSample/actions/workflows/release.yml) [![Apache-2.0](https://img.shields.io/github/license/Ivan-VHS/VentuSample)](LICENSE)

> **Deterministic sample generation and offline DSP. No pretend real-time engine.**

**State:** Development · offline audio core implemented · Qt desktop partial · **Version:** 0.2.0 · **License:** Apache-2.0 · **README standard:** Psychoporter 666 v1.1 · **Last reviewed:** 2026-11-25 UTC · **Source:** [Ivan-VHS/VentuSample](https://github.com/Ivan-VHS/VentuSample)

<p align="center"><img src="assets/readme/dashboard.svg" alt="VentuSample project overview" width="100%"></p>
<p align="center"><a href="assets/screenshots/ventusample-studio.png"><img src="assets/screenshots/ventusample-studio.png" alt="VentuSample desktop application screenshot" width="100%"></a></p>

## Quick navigation

[🚀 Demo: Studio screenshot](assets/screenshots/ventusample-studio.png) · [Documentation](docs/README.md) · [Source](https://github.com/Ivan-VHS/VentuSample) · [Issues](https://github.com/Ivan-VHS/VentuSample/issues)

## Mission & scope

A native procedural sample workstation with a C++20 core, a C11 DSP compatibility layer and an optional Qt 6 Widgets desktop application. Rendering is offline. It is not a real-time DAW, VST/AU host, cloud service or hardware audio driver.

## Capabilities

- Seeded procedural stereo generation across 14 documented hip-hop and adjacent profiles.
- Golden-era sample-crate reassembly, weighted drum voicing and deterministic variations.
- Bounded PCM16 WAV import/export, four-slice placement and semitone resampling.
- Offline DSP graph routing with bounded wet/feedback, RMS matching and sample-peak guard.
- Versioned ventusample.pack.v1 and v2 manifests for explicit pack exchange.
- Optional Qt 6 Widgets workstation; desktop compilation depends on Qt SDK availability.

## Architecture & data flow

~~~mermaid
flowchart LR
  P["Seed / profile / parameters"] --> G["Deterministic generator"]
  G --> D["Offline C11 DSP layer"]
  D --> X["C++20 audio core"]
  X --> W["PCM16 WAV assets"]
  X --> M["Versioned pack manifest"]
  Q["Optional Qt 6 UI"] --> P
~~~

The generator sends seeded settings through the offline DSP core into bounded WAV assets and a versioned pack manifest. The optional Qt interface configures inputs; it does not turn the engine into a real-time audio host.

## Requirements

CMake 3.21+, a C++20 compiler and a C compiler for the C11 DSP layer. Qt 6 Widgets is optional for the desktop app; the portable core and tests can be built without Qt. Ninja is recommended on Linux.

## Build

~~~sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=ON -DVENTU_BUILD_TESTS=ON
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
~~~

### Portable core without Qt

~~~sh
cmake -S . -B build-core -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=OFF -DVENTU_BUILD_TESTS=ON
cmake --build build-core --parallel
ctest --test-dir build-core --output-on-failure
~~~

## Run

The desktop target is generated only when Qt 6 Widgets is found; otherwise use the portable core build.

## Interfaces

| Surface | Contract | Boundary |
|---|---|---|
| Presets | `ventusample.preset.v1` | Schema-validated settings |
| Pack exchange | `ventusample.pack.v1 / v2` | JSON manifest plus WAV assets |
| Audio output | `RIFF PCM16 stereo WAV` | Bounded local file output with peak/RMS metadata |
| Plugin layer | Native offline DSP graph | No real-time VST/AU hosting |

## Security & trust boundaries

Rendering is offline by default; no background audio capture or network telemetry is part of the audio pipeline. Imported WAV files are untrusted and bounded by format, chunk, frame and size checks. External native libraries are not loaded implicitly. No driver, firmware or system-audio control is performed.

Imported assets must be treated as untrusted data. Validate file sizes, chunk boundaries and declared formats before consuming them.

## Verification

~~~sh
ctest --test-dir build --output-on-failure
~~~

The test targets exercise deterministic generation, plugin routing, WAV boundaries, resampling, MIDI files, offline spectrum analysis and runtime transactions. Desktop UI tests are registered only when Qt Widgets is available. Coverage and fuzzing percentages are not claimed without published reports.

## Performance evidence

Generator and DSP tests validate deterministic and bounded behavior, not real-time playback latency. No fixed microsecond latency, RAM footprint or supported sample-rate matrix is claimed here.

## Compatibility

CMake exposes a portable core and an optional Qt 6 Widgets desktop target. MSVC and GCC/Clang build routes are documented; desktop availability depends on a compatible Qt 6 SDK.

## Documentation & ecosystem

- [Features](docs/FEATURES.md)
- [CRATE Hip-Hop Production Suite](docs/CRATE_PRODUCTION_SUITE.md)
- [Hip-Hop Preset Crate](examples/hip-hop-presets/README.md)
- [Architecture](docs/ARCHITECTURE.md)
- [Quantum Audio Engine](docs/QUANTUM_AUDIO_ENGINE.md)
- [MIDI and resampling](docs/MIDI_AND_RESAMPLING.md)
- [Plugins](docs/PLUGINS.md)
- [Source integration and licensing](docs/SOURCE_INTEGRATION.md)
- [Documentation index](docs/README.md)

The audio-engine, MIDI/resampling, plugin and source-licensing documents define format boundaries and extension points.

Reaper pack exchange is file-based and versioned. Optional Reaper native expression integration is enabled only when a local Reaper source tree is explicitly supplied to CMake; Pytyliki native metrics remain a planned opt-in path.

## Contributing

Read [CONTRIBUTING.md](CONTRIBUTING.md) before changing code or format contracts. Keep changes reviewable, add deterministic tests and use Conventional Commits where required.

## License & legal

Distributed under Apache-2.0. The software is provided “AS IS” under the license terms. Third-party names and marks remain with their owners.

Hosted-service terms, cookies, CORS and OAuth are not applicable to the documented local runtime.

---

<p align="center"><sub>Psychoporter 666 README Standard v1.0 · Ivan-VHS · 2026-11-25 UTC</sub></p>
