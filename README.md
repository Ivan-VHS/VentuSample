# VentuSample — Procedural Sample Workstation

[English](README.md) · [Українська](README.uk.md) · [Русский](README.ru.md) · [简体中文](README.zh-CN.md)

[![CI](https://img.shields.io/github/actions/workflow/status/Ivan-VHS/VentuSample/ci.yml?branch=main&label=CI)](https://github.com/Ivan-VHS/VentuSample/actions/workflows/ci.yml) [![Apache-2.0](https://img.shields.io/github/license/Ivan-VHS/VentuSample)](LICENSE)

<p align="center"><img src="assets/readme/dashboard.svg" alt="VentuSample project overview" width="100%"></p>
<p align="center"><a href="assets/screenshots/ventusample-studio.png"><img src="assets/screenshots/ventusample-studio.png" alt="VentuSample desktop application" width="100%"></a></p>

VentuSample is a native procedural sample workstation built around a C++20/Qt 6 desktop application and an offline audio-processing core.

**Scope:** local sound generation, parameter editing, offline DSP and file export. It is not a real-time DAW, VST/AU host, cloud service or hardware audio driver.

## Capabilities

- Procedural sample generation, presets and in-session A/B parameter slots.
- Offline DSP, WAV import/export, MIDI and sample-rate conversion tools.
- Plugin SDK and versioned pack manifests; discovery and interchange follow explicit metadata and trust boundaries.
- Reproducible generation settings and bounded file handling.

See [Features](docs/FEATURES.md), [Architecture](docs/ARCHITECTURE.md), [Quantum Audio Engine](docs/QUANTUM_AUDIO_ENGINE.md), [MIDI and resampling](docs/MIDI_AND_RESAMPLING.md), [Plugins](docs/PLUGINS.md) and [Source integration](docs/SOURCE_INTEGRATION.md).

## Build and test

Requirements: CMake, a C++20 compiler and Qt 6 Widgets for the desktop application. The portable core can be built without Qt.

Linux:

```sh
sudo apt-get update
sudo apt-get install -y cmake ninja-build g++ qt6-base-dev
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=ON -DVENTU_BUILD_TESTS=ON
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
```

Portable core without Qt:

```sh
cmake -S . -B build-core -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=OFF -DVENTU_BUILD_TESTS=ON
cmake --build build-core --parallel
ctest --test-dir build-core --output-on-failure
```

Windows uses MSVC and Qt 6:

```sh
cmake -S . -B build -A x64 -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=ON -DVENTU_BUILD_TESTS=ON
cmake --build build --config Release --parallel 2
ctest --test-dir build -C Release --output-on-failure
```

## Specifications and integrations

- Preset format: [ventusample.preset.v1 schema](schemas/ventusample.preset.v1.schema.json).
- [Documentation index](docs/README.md) · [Reaper pack audit](docs/REAPER_PACK_AUDIT.md) · [Release process](docs/RELEASE.md).
- [Shared ecosystem standards](https://github.com/Ivan-VHS/DuckOS/blob/main/docs/SHARED_STANDARDS.md)
- [Project Evolution Ladder](docs/PROJECT_LADDER.md) · [Machine-readable rung](.github/project-ladder.v1.json).

[Contributing](CONTRIBUTING.md) · [Security policy](SECURITY.md) · [Apache-2.0](LICENSE)
