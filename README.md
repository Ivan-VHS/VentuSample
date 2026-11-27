# VentuSample — offline procedural audio workstation

[English](README.md) · [Українська](README.uk.md) · [Русский](README.ru.md) · [简体中文](README.zh-CN.md)

[![CI](https://img.shields.io/github/actions/workflow/status/Ivan-VHS/VentuSample/ci.yml?branch=main&label=CI)](https://github.com/Ivan-VHS/VentuSample/actions/workflows/ci.yml) [![Release workflow](https://img.shields.io/github/actions/workflow/status/Ivan-VHS/VentuSample/release.yml?branch=main&label=Release)](https://github.com/Ivan-VHS/VentuSample/actions/workflows/release.yml) [![Apache-2.0](https://img.shields.io/github/license/Ivan-VHS/VentuSample)](LICENSE)

> Deterministic C++20 sample generation and offline DSP with bounded WAV I/O, reproducible routing and versioned sample-pack manifests.

**Project record:** Development · version 0.2.0 · C++20 core with C11 DSP compatibility layer · optional Qt 6 Widgets desktop · offline rendering · Apache-2.0 · **Documentation reviewed:** 2026-11-27 UTC

<p align="center"><img src="assets/screenshots/ventusample-studio.png" alt="VentuSample offline workstation" width="100%"></p>

## Purpose and architecture

VentuSample creates reproducible sample packs and processes audio files locally. The C++20 core owns parameter validation, seeded pattern generation, stereo processing, offline DSP routing, WAV import/export and manifest generation. The C11 compatibility layer provides low-level DSP and MIDI/file primitives. The optional Qt 6 Widgets application provides a desktop surface; it is not a real-time audio host.

~~~mermaid
flowchart TB
  U["Profile / seed / parameters"] --> V["Validation and resource bounds"]
  V --> G["Pattern scheduler and renderer"]
  G --> F["Stereo float processing"]
  F --> D["Offline DSP and stochastic routing"]
  D --> M["Master / RMS and peak guard"]
  M --> W["Bounded PCM16 WAV assets"]
  V --> J["Versioned pack manifest v1/v2"]
  W --> P["Local sample pack"]
  J --> P
  Q["Optional Qt 6 desktop"] --> U
~~~

## Capabilities

- **Procedural generation:** seeded stereo patterns across 14 documented hip-hop and adjacent profiles, including the editable Billionaire Golden Era profile.
- **Golden-era crate workflow:** deterministic four-slice 0–2–1–3 reassembly, weighted kick/snare voicing, restrained hats, shared swing and repeatable turnarounds.
- **WAV import and chop:** bounded PCM16 mono/stereo input (8–192 kHz, up to 128 MiB and 20 million frames), four-slice placement and semitone resampling over ±12 semitones. Pitch changes also change playback speed; tempo-preserving time-stretch is not implemented.
- **Offline DSP:** filters, saturation, linked stereo-bus processing, RMS matching, sample-peak guarding and bounded effect routing.
- **Stochastic router:** eight weighted effects across serial, parallel and hybrid graphs; file, bar and generator-onset triggers; deterministic seed/variation routing; bounded wet/feedback, fades and output guards. Integrated into offline export.
- **Analysis:** per-file peak, RMS and crest factor; offline FFT peak, centroid and 85% rolloff. No audio-device access is needed.
- **Tonal metadata:** key/mode labels are attached to pitched material only; unpitched drums are not assigned fabricated keys.
- **Pack exchange:** versioned `ventusample.pack.v1` and `ventusample.pack.v2` manifests with WAV assets. Reaper's current consumer validates and compares v1 manifest metadata; it does not audit or modify waveform data.

The profiles are original procedural patterns, not recordings or claims to reproduce a specific artist. Imported audio remains the user's responsibility; the software does not establish copyright or licensing clearance.

## Build and verify

Requirements: CMake 3.21+, a C++20 compiler and a C compiler for the C11 DSP layer. Qt 6 Widgets is optional. The portable core and tests can be built without Qt.

~~~sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=ON -DVENTU_BUILD_TESTS=ON
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
~~~

Portable core without Qt:

~~~sh
cmake -S . -B build-core -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=OFF -DVENTU_BUILD_TESTS=ON
cmake --build build-core --parallel
ctest --test-dir build-core --output-on-failure
~~~

The desktop target is generated only when Qt 6 Widgets is found. Without the SDK, the portable core and available tests remain buildable.

## Audio and file boundaries

| Surface | Contract | Boundary |
|---|---|---|
| Presets | `ventusample.preset.v1` | Validated local settings |
| Pack exchange | `ventusample.pack.v1 / v2` | JSON manifest and WAV assets |
| Audio output | RIFF PCM16 stereo WAV | Bounded local render with peak/RMS metadata |
| DSP graph | Native offline DSP | No real-time VST/AU hosting |
| Analysis | Offline FFT and file metrics | In-memory/local data; no audio-device capture |

Rendering is offline. The project does not currently provide real-time playback, an audio-device engine, a VST/AU host, tempo-preserving time-stretch or GPU dispatch. No fixed real-time latency, RAM footprint or universal sample-rate performance claim is made.

## Tests and performance evidence

CTest covers deterministic generation, WAV boundaries, resampling, MIDI files, offline spectrum analysis, plugin routing and runtime transactions. Qt-dependent UI tests are registered only when Qt 6 Widgets is available. Coverage or fuzzing percentages are not claimed without published reports.

The tests establish deterministic and bounded behavior, not real-time playback latency or a hardware-independent performance ranking.

## Interoperability and documentation

Reaper pack exchange is file-based. The current Reaper consumer reads v1 manifest metadata and leaves WAV assets untouched. Optional Reaper native expression integration is enabled only when a local Reaper source tree is explicitly supplied to CMake. A direct Pytyliki native metrics/DSP adapter remains planned.

- [Feature matrix](docs/FEATURES.md) · [Architecture](docs/ARCHITECTURE.md)
- [CRATE production suite](docs/CRATE_PRODUCTION_SUITE.md) · [Hip-hop preset crate](examples/hip-hop-presets/README.md)
- [Quality Monolith Rhythm](docs/QUALITY_MONOLITH_RHYTHM.md) · [Quantum Audio Engine](docs/QUANTUM_AUDIO_ENGINE.md)
- [Plugins and lifecycle](docs/PLUGINS.md) · [MIDI and resampling](docs/MIDI_AND_RESAMPLING.md)
- [Source integration and licensing](docs/SOURCE_INTEGRATION.md) · [Documentation index](docs/README.md)
- [Documentation control](docs/DOCUMENTATION_CONTROL.md)

## Security, contribution and license

Treat imported WAV files and preset packs as untrusted input. Validate file size, RIFF chunks, frame counts and declared formats before processing. External native libraries are not loaded implicitly. No driver, firmware or system-audio control is performed.

Read [CONTRIBUTING.md](CONTRIBUTING.md) before changing DSP behavior, file formats or plugin contracts. The project is licensed under [Apache-2.0](LICENSE); preserve third-party notices.
