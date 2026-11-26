# VentuSample — offline procedural audio workstation

[English](README.md) · [Українська](README.uk.md) · [Русский](README.ru.md) · [简体中文](README.zh-CN.md)

> A deterministic C++20 sample-generation and offline DSP workstation with bounded PCM16 WAV import/export, versioned pack manifests and an optional Qt 6 desktop shell.

**Specification**

| Field | Current record |
|---|---|
| Repository | [Ivan-VHS/VentuSample](https://github.com/Ivan-VHS/VentuSample) |
| Project version | 0.2.0 (project contract and CMake) |
| Project status | Development; offline audio core |
| Core | C++20; C11 DSP compatibility layer |
| Desktop | Optional Qt 6 Widgets shell |
| Build | CMake 3.21+ |
| Audio output | PCM16 WAV, deterministic local render |
| Pack formats | Manifest v1 and v2 |
| License | [Apache-2.0](LICENSE) |
| Code of Conduct | Not published (no root `CODE_OF_CONDUCT.md`) |
| Last documentation review | 2026-11-26 |

<p align="center"><img src="assets/screenshots/ventusample-studio.png" alt="VentuSample offline workstation" width="100%"></p>

## Contents
- [Purpose and architecture](#purpose-and-architecture)
- [Build and verification](#build-and-verification)
- [Audio and file contracts](#audio-and-file-contracts)
- [DSP and plugin system](#dsp-and-plugin-system)
- [Interoperability](#interoperability)
- [Limitations and rights](#limitations-and-rights)
- [Documentation and versioning](#documentation-and-versioning)

## Purpose and architecture

VentuSample generates reproducible sample packs and processes audio files locally. The C++20 core owns parameter validation, deterministic pattern generation, stereo float processing, offline DSP routing, WAV I/O and manifest generation. A Qt 6 Widgets desktop target is optional and is built only when the SDK is available.

~~~mermaid
flowchart TB
  U["Parameters / seed / variation"] --> V["Validation and resource bounds"]
  V --> G["Pattern scheduler and renderer"]
  G --> F["Stereo float buffer"]
  F --> D["Offline DSP / stochastic router"]
  D --> M["Master and peak guard"]
  M --> W["PCM16 WAV assets"]
  V --> J["Versioned pack manifest v1/v2"]
  W --> P["Pack directory"]
  J --> P
  Q["Optional Qt 6 desktop"] --> U
~~~

The renderer runs offline on a worker thread. No real-time audio callback or audio-device access is provided.

## Build and verification

Requirements: CMake 3.21+ and a C++20 compiler. Install Qt 6 Widgets if you need the desktop executable.

### Core and available tests

~~~sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=ON -DVENTU_BUILD_TESTS=ON
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
~~~

Without Qt 6, build the core and its tests explicitly:

~~~sh
cmake -S . -B build-core -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=OFF -DVENTU_BUILD_TESTS=ON
cmake --build build-core --parallel
ctest --test-dir build-core --output-on-failure
~~~

Qt UI tests are available only when the Qt Widgets target is configured. A skipped desktop target is not a successful desktop build.

## Audio and file contracts

| Surface | Contract |
|---|---|
| Generated assets | RIFF/WAVE PCM16 |
| Imported source | PCM16 mono or stereo WAV |
| Source rate | 8–192 kHz |
| Source bounds | ≤128 MiB and ≤20,000,000 frames |
| Chopping | Four-slice import and beat placement |
| Pitch shift | ±12 semitones by resampling; duration/speed changes |
| Export sample rates | 22.05, 44.1, 48 or 96 kHz |
| Pack manifests | v1 and v2; v2 adds optional tonal and routing metadata |
| Reaper audit | Reads versioned manifest metadata; WAV files remain untouched |

All imported audio is untrusted input and must pass format, chunk, frame and size validation. Tonal labels represent generator settings; they are not automatic key detection of imported material. Drum assets are not assigned fabricated key labels.

## DSP and plugin system

The current project includes seeded procedural style profiles, a shared swing/timing schedule, Golden Era phrase arrangement, bounded stereo mastering, an eight-effect stochastic offline router and a Quantum Audio Engine catalogue with 133 named module identities built from 21 reusable DSP kernels.

The stochastic router operates on rendered audio buffers before WAV encoding. Its onset trigger uses renderer-scheduled event frames; it does not detect transients in arbitrary imported audio. The Quantum Audio Engine is an offline DSP catalogue and processing chain, not a physical quantum computer or a real-time DAW engine.

See [Features](docs/FEATURES.md), [Architecture](docs/ARCHITECTURE.md), [Plugins](docs/PLUGINS.md), [Stochastic/quality workflow](docs/QUALITY_MONOLITH_RHYTHM.md) and [Quantum Audio Engine](docs/QUANTUM_AUDIO_ENGINE.md).

## Interoperability

VentuSample exports manifest v1/v2 plus WAV assets. Reaper's current consumer validates and compares pack metadata; it does not analyze, rewrite or render the WAV waveform. The direct Pytyliki native metrics adapter remains planned and must be opt-in with ABI differential tests before being documented as available.

The Reaper bridge in CMake is disabled unless a local Reaper source directory is explicitly supplied. No external native library is loaded implicitly.

## Limitations and rights

> VentuSample is an offline workstation, not a real-time audio engine or DAW plugin host.

Real-time playback, audio-device capture, DAW plugin hosting, tempo-preserving time-stretch and GPU dispatch are not implemented. Source transposition uses resampling, so pitch changes also change playback duration. The renderer uses a portable scalar path; no universal latency or throughput is claimed.

Users remain responsible for rights to imported samples, generated outputs and distributed packs. The repository does not assert ownership or license clearance for third-party audio material. Do not include private sample paths or unlicensed source recordings in public fixtures.

## Documentation and versioning

Start with [Documentation Index](docs/README.md) and [Information Architecture](docs/INFORMATION_ARCHITECTURE.md).

- [Feature matrix](docs/FEATURES.md)
- [Architecture and audio contract](docs/ARCHITECTURE.md)
- [User guide](docs/USER_GUIDE_RU.md)
- [Manifest and Reaper pack audit](docs/REAPER_PACK_AUDIT.md)
- [Plugin API and lifecycle](docs/PLUGINS.md)
- [MIDI and resampling](docs/MIDI_AND_RESAMPLING.md)
- [Quality Monolith Rhythm](docs/QUALITY_MONOLITH_RHYTHM.md)
- [Quantum Audio Engine](docs/QUANTUM_AUDIO_ENGINE.md)
- [Documentation control](docs/DOCUMENTATION_CONTROL.md)

The project contract and CMake declare version 0.2.0; that does not by itself establish a stable release. Public pack-format changes require schema updates, compatibility tests and migration notes. Read [CONTRIBUTING.md](CONTRIBUTING.md) and [SECURITY.md](SECURITY.md) before changing DSP input handling, plugins or pack contracts. The project uses [Apache-2.0](LICENSE) and preserves bundled third-party notices.