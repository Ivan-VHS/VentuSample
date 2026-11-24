# VentuSample

VentuSample is a native C++20 workstation for procedural hip-hop sample-pack generation and offline WAV processing, with an optional Qt 6 desktop interface.

[English](README.md) · [Українська](README.uk.md) · [Русский](README.ru.md) · [简体中文](README.zh-CN.md)

<picture><source media="(prefers-color-scheme: dark)" srcset="assets/readme/dashboard.svg"><img src="assets/readme/dashboard-light.svg" alt="VentuSample overview: parameter-driven C++20 rendering, offline DSP and local sample-pack export" width="100%"></picture>

[![C++20 CI](https://img.shields.io/github/actions/workflow/status/Ivan-VHS/VentuSample/ci.yml?branch=main&style=flat-square&label=C%2B%2B20%20CI)](https://github.com/Ivan-VHS/VentuSample/actions/workflows/ci.yml) [![Last commit](https://img.shields.io/github/last-commit/Ivan-VHS/VentuSample?style=flat-square)](https://github.com/Ivan-VHS/VentuSample/commits/main) [![Apache-2.0](https://img.shields.io/github/license/Ivan-VHS/VentuSample?style=flat-square)](LICENSE)

[Quick Start](#quick-start) · [Screenshots](#screenshots) · [Architecture](#architecture) · [Roadmap](#roadmap) · [Docs](docs/README.md) · [Commits](https://github.com/Ivan-VHS/VentuSample/commits/main)

> **Status:** Active development. Native C++20 generation core, C11 DSP support library, deterministic tests and optional Qt 6 desktop application are in the repository. CI builds Linux core/Qt and Windows desktop targets.
>
> **Requirements:** CMake 3.21+, C++20 compiler, C11 compiler and Git. Qt 6 Widgets is required for the desktop interface; disable the desktop target for a portable core build without Qt.

## Reaper integration

VentuSample optionally links Reaper's versioned C17 ABI for bounded integer-expression evaluation and exact selfhood lookup. This integration is control-language infrastructure only: waveform generation, sample placement, stochastic routing, offline DSP, audio buffers, Qt and export remain native C++20. Reaper is not executed on a real-time audio path and is not a general-purpose plugin loader. See [Reaper runtime integration](docs/REAPER_RUNTIME_INTEGRATION.md).

## Quick Start

Linux with Qt 6:

~~~sh
sudo apt-get update
sudo apt-get install -y cmake ninja-build g++ qt6-base-dev
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=ON -DVENTU_BUILD_TESTS=ON
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
~~~

Portable DSP/core build without Qt:

~~~sh
cmake -S . -B build-core -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=OFF -DVENTU_BUILD_TESTS=ON
cmake --build build-core --parallel
ctest --test-dir build-core --output-on-failure
~~~

Windows with Visual Studio 2022 and Qt 6 configured:

~~~powershell
cmake -S . -B build -A x64 -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=ON -DVENTU_BUILD_TESTS=ON
cmake --build build --config Release --parallel 2
ctest --test-dir build -C Release --output-on-failure
~~~

## What it does

| Area | Implemented behavior | Boundary |
|---|---|---|
| Sample-pack generator | Full Loop, Melody, Drums, Bass and One-Shot Kit outputs | Local procedural generation from explicit settings and seed |
| Audio export | Stereo PCM16 WAV and versioned JSON manifest | Writes local files; does not overwrite existing outputs |
| Source audio | Imports supported PCM16 mono/stereo WAV files | Bounded format, frame-count, sample-rate and size validation |
| Quantum Audio Engine | 133 typed module identities and 21 reusable processing kernels | Offline DSP catalogue; not physical quantum computation |
| C11 DSP support | Filters, compressor, saturation, limiter, resampling and spectral utilities | Separate bounded native C11 library |
| Spectrum analysis | FFT peak, spectral centroid and 85% rolloff | Existing buffers only; bounded 256–65,536 point FFT |
| MIDI inspection | Reads Standard MIDI File format 0/1, notes, tempo and time signature | Read-only parser; no MIDI device or playback |
| Stochastic Router | Optional seeded routing variation | Reproducible when settings and seed are fixed |
| Presets | Local schema version 1 presets and in-session A/B slots | Local source-sample paths are not exported |
| Plugin tooling | 19 built-in tools and metadata-only plugin discovery | Arbitrary DLL/SO plugins are not loaded |
| Reaper bridge | Optional native C17 integer-expression ABI | Requires explicit local Reaper source checkout |

VentuSample is **not** a DAW, real-time audio engine, audio-device host, browser sandbox or hosted service. Rendering and DSP operate on local buffers and files. Atomic-Quantum is a classical simulation feature, not quantum hardware.

## Why VentuSample

- Deterministic, seed-aware sample generation with local, inspectable manifests.
- A C++20 product core and separately testable C11 DSP support library.
- Offline processing avoids hidden audio-device dependencies and real-time scheduling claims.
- Versioned presets support A/B comparison while omitting machine-specific source paths.
- Optional integrations are explicitly gated; no arbitrary native plugin binary is loaded.

## Screenshots

<picture><source media="(prefers-color-scheme: dark)" srcset="assets/readme/workflow-dark.svg"><img src="assets/readme/workflow.svg" alt="VentuSample data path from parameters through C++20 rendering and offline processing to a WAV pack and manifest" width="100%"></picture>

<p align="center"><a href="assets/screenshots/ventusample-studio.png"><img src="assets/screenshots/ventusample-studio.png" alt="Real Windows capture of the VentuSample Qt sample workstation" width="100%"></a></p>
<sub>1920×1080 Windows desktop capture created by the native CI workflow. The diagram above is a vector architecture illustration, not a screenshot.</sub>

## Architecture

~~~mermaid
flowchart LR
 A[Parameters and seed] --> B[C++20 generator]
 B --> C[Stereo float buffer]
 C --> D[Offline C11 DSP]
 D --> E[PCM16 WAV and JSON manifest]
 F[Optional Qt 6 desktop] --> A
 G[Optional Reaper C17 bridge] --> B
~~~

| Path | Responsibility |
|---|---|
| src/ | C++20 generator, DSP integration, presets and Qt desktop |
| src/compat/pytyliki-dsp/ | C11 DSP support library |
| tests/ | Generator, DSP, preset and contract tests |
| schemas/ | Versioned pack and preset JSON schemas |
| docs/ | Audio, architecture, UI, quality and integration contracts |
| assets/screenshots/ | Captured Qt desktop evidence |
| .github/workflows/ | Python contract checks, native tests and Qt desktop builds |

## Presets and A/B

Presets use the versioned local JSON format. They store the sound profile and generation/DSP settings; the random seed is omitted by default unless the user explicitly includes it. The two in-session A/B slots switch between parameter snapshots without writing files. Source sample paths are excluded because they are machine-specific.

Schema: [Preset v1](schemas/ventusample.preset.v1.schema.json). Presets are local files; there is no cloud sync or claim of ownership over imported or generated audio.

## Compatibility

| Target | Requirements | CI / scope |
|---|---|---|
| Linux core and desktop | CMake 3.21+, C++20/C11 toolchains, Qt 6 Widgets for UI | Ubuntu 24.04, Qt 6 |
| Windows desktop | Visual Studio 2022, CMake 3.21+, Qt 6 Widgets | Windows Server 2022 runner, Qt 6.8.3 |
| Portable core | CMake 3.21+, C++20/C11 toolchains | Set VENTU_BUILD_DESKTOP=OFF |
| Python tooling | Python 3.10–3.12 | Engineering and contract checks only |
| Reaper bridge | Optional local Reaper source tree | Set REAPER_SOURCE_DIR explicitly |

The desktop target is optional. If Qt 6 Widgets is not available, configure with VENTU_BUILD_DESKTOP=OFF to build the portable core and tests.

## Revival highlights

- First-screen summary now gives the product, maturity, toolchain and exact build/test path.
- The live Windows capture is distinct from the two-mode architecture artwork.
- Local light/dark SVG pairs have equal canvas dimensions and descriptive alt text.
- Offline scope, optional desktop dependency, preset behavior and native plugin limits are stated directly.

| Previous README flow | Updated README flow |
|---|---|
| Overview and preset notes before build instructions | Working build/test command in the opening section |
| Visuals concentrated in one theme | Adaptive, local SVG artwork |
| Optional dependencies discovered late | Qt and Reaper bridge called out up front |
| Feature descriptions without boundary table | Implementation and limits side by side |

## Roadmap

- [x] C++20 generator core and C11 DSP support library.
- [x] Offline WAV export, JSON manifests and source-WAV validation.
- [x] Qt 6 desktop workflow and deterministic native test targets.
- [x] Versioned local presets and in-session A/B slots.
- [ ] Expand rendering/DSP modules with deterministic reference fixtures and numerical regression checks.
- [ ] Add additional formats only with versioned schemas, bounded parsers and compatibility tests.

## Ecosystem

- [DuckOS](https://github.com/Ivan-VHS/DuckOS) — operating-system project and Mora workspace.
- [Reaper](https://github.com/Ivan-VHS/Reaper) — systems language and optional native expression bridge.
- [Pytyliki](https://github.com/Ivan-VHS/Pytyliki) — processor inspection and systems utilities.
- [OPlague](https://github.com/Ivan-VHS/OPlague) — device and driver evidence workbench.

See [documentation](docs/README.md), [architecture](docs/ARCHITECTURE.md), [visual publishing standard](docs/README_VISUAL_STANDARD.md), [contribution guide](CONTRIBUTING.md) and [issues](https://github.com/Ivan-VHS/VentuSample/issues). License: [Apache-2.0](LICENSE).
