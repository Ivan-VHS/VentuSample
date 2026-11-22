# VentuSample — procedural sample workstation

[English](README.md) · [Українська](README.uk.md) · [Русский](README.ru.md) · [简体中文](README.zh-CN.md)

<p align="center"><img src="assets/readme/dashboard.svg" alt="VentuSample project overview" width="100%"></p>

[Source](https://github.com/Ivan-VHS/VentuSample) · [CI workflow](.github/workflows/ci.yml) · [Documentation index](docs/README.md) · [Apache-2.0](LICENSE)

> **Scope:** VentuSample is a native C++20 workstation for deterministic, procedural sample-pack generation and offline WAV processing. It has a Qt 6 desktop interface when Qt is available and a portable DSP/core build. It is not a hosted web demo, browser sandbox, DAW, audio-device host or production audio service. Documentation was checked against `main` on **2026-11-22**.

## Purpose and boundaries

VentuSample creates local hip-hop sample packs from explicit parameters and a seed. It can render audio, process supported WAV files through offline DSP, and write versioned JSON manifests describing the settings and outputs. It does not need a DAW, audio interface, network connection or hosted backend.

The intended audience is beatmakers, sound designers, audio developers, QA engineers and reviewers who need deterministic, inspectable sample generation and repeatable offline processing.

### What is implemented

| Area | Implemented behavior | Boundary |
|---|---|---|
| Procedural generator | Full Loop, Melody, Drums, Bass and One-Shot Kit outputs | local rendering; fixed seed/settings reproduce the same render |
| Audio export | stereo PCM16 WAV and versioned JSON manifest | local files; existing output files are not overwritten |
| WAV import | supported PCM16 mono/stereo source files | validated size, frame count, sample rate and format |
| Quantum Audio Engine | 133 typed module identities and 21 reusable DSP kernels | creative offline DSP, not physical quantum computation |
| Stochastic Router | optional seeded routing through eight built-in processors | offline buffer processing; disabled by default |
| Plugin Manager | 19 built-in tools and a reviewed C++20 handler API | manifest discovery is metadata-only; no arbitrary DLL/SO loading |
| Atomic-Quantum 133 | separate bounded classical simulation subset | no quantum hardware access |
| Reaper bridge | optional C17 integer-expression ABI at build time | explicit local source path; not part of the audio DSP path |

The 14 generator profiles are Boom Bap, Billionaire Golden Era, East Coast, Harlem Dip-Trap, G-Unit, Modern Trap, Atlanta Trap, UK Drill, UK Grime, Memphis, Jazz Rap, G-Funk, Boom Trap and Cripple Trill.

The application is not a real-time audio engine. Rendering and DSP run offline on worker threads; there is no audio-device callback, playback hot-swap, VST/AU host, arbitrary third-party plugin loader or DAW project automation.

The Windows screenshot below is a capture of the desktop application, not a browser preview.

<p align="center"><img src="assets/screenshots/ventusample-studio.png" alt="VentuSample running on Windows 11" width="100%"></p>
<sub>Desktop capture · 1920×1080 source image. This is not a hosted web stand.</sub>

## Quick Start

### Requirements

- CMake **3.21 or newer**.
- A C++20 compiler: GCC, Clang or MSVC.
- Git.
- Qt **6 Widgets** for the desktop application. The DSP/core library and non-Qt tests can still be built when Qt is unavailable.
- Ninja is optional, but is used by the Linux CI desktop job.

The project does not require Python or Node.js to render audio. Python 3.10–3.12 is used by CI for project-contract and engineering validation scripts.

### Linux: build the desktop workstation

On Ubuntu 24.04, install the same toolchain packages used by CI:

```sh
sudo apt-get update
sudo apt-get install -y cmake ninja-build g++ qt6-base-dev
```

Configure, build and run tests:

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=ON -DVENTU_BUILD_TESTS=ON
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
```

The desktop executable is produced by the configured CMake build. For the Ninja Linux build, launch it with `./build/VentuSample`; for the Windows Release build, launch `build\\Release\\VentuSample.exe`.

### Build the portable core without Qt

When Qt 6 Widgets is not installed, or when you only need the DSP engine and tests:

```sh
cmake -S . -B build-core -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=OFF -DVENTU_BUILD_TESTS=ON
cmake --build build-core --parallel
ctest --test-dir build-core --output-on-failure
```

If `VENTU_BUILD_DESKTOP=ON` but Qt 6 Widgets cannot be found, CMake prints a warning and skips the desktop target; this is not a claim that a GUI executable was produced.

### Windows: build with MSVC and Qt 6

Use a Visual Studio 2022 x64 developer environment and install Qt 6 Widgets (the CI job currently uses Qt **6.8.3**, MSVC 2022 x64):

```sh
cmake -S . -B build -A x64 -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=ON -DVENTU_BUILD_TESTS=ON
cmake --build build --config Release --parallel 2
ctest --test-dir build -C Release --output-on-failure
```

The Windows CI runner installs Qt and configures the matching MSVC toolchain before running these commands.

### First smoke check

A successful `ctest` run is the general build check. The tests cover parameter validation, deterministic repeatability, variations, WAV round-trips, stochastic DSP, schemas, plugin lifecycle and Quantum Audio Engine behavior.

There is no default HTTP port, localhost URL, browser launch command, database seed or remote mock service. All generation and processing are local.

## Reproducible data and test scenarios

VentuSample does not use a remote database or live API. The generator uses local parameter values and a seed; imported audio comes from a file selected by the operator. Manifests are generated locally alongside output files.

Try the following scenarios in the desktop application:

1. **Repeatable loop:** choose a generator profile, BPM, key/scale and fixed seed; render a Full Loop twice with identical settings and compare the resulting audio/manifests.
2. **Separate stems:** render Melody, Drums and Bass independently and inspect each output type.
3. **One-shot kit:** generate the eight-hit kit and verify that tonal labels are only applied to pitched sounds.
4. **WAV import:** load a supported PCM16 mono/stereo file and inspect its source properties.
5. **Chop and pitch:** use the source-chop controls to select up to four slices and apply the documented semitone resampling; this changes pitch and playback speed together, not time-stretch.
6. **Offline DSP chain:** load a supported WAV in Quantum Audio Engine, arrange the ordered processing chain, adjust module controls and export the processed WAV.
7. **Stochastic routing:** enable the optional router, choose topology/trigger/intensity and render again. The seed, variation, trigger and effect mask determine the route.
8. **Plugin discovery:** inspect a local `plugin.json`; discovered metadata remains disabled until a reviewed native handler is explicitly attached.

### Supported generator ranges and file bounds

| Control / input | Supported range |
|---|---|
| BPM / bars / variations | 60–190 / 1–16 / 1–32 |
| Key / scale | C–B / 6 scale families |
| Swing | 50–68% |
| Density / grit / saturation / stereo | 0–100% |
| High-pass / low-pass | 20–300 Hz / 2–20 kHz |
| Snare offset | −30…+30 ms |
| Bus compression | 1:1–12:1 |
| Sample rates | 22.05 / 44.1 / 48 / 96 kHz |
| Imported source | PCM16 mono/stereo, 8–192 kHz |
| Source-file limits | 128 MiB and 20,000,000 frames |
| Chop pitch / mix | −12…+12 semitones / 0–100% |

The low-pass cutoff must exceed the high-pass cutoff and remain below 0.49× the sample rate. Imported audio is not automatically key-detected.

### Stochastic Router contract

The router is optional and **off by default**. It processes rendered stereo buffers before PCM16 WAV encoding. The built-in pool contains low-pass, high-pass, saturator, bitcrusher, delay, reverb, ring modulation and stereo width.

| Parameter | Contract |
|---|---|
| Topology | hybrid, serial or parallel; parallel uses 2–4 bounded branches |
| Trigger | per file, per bar or renderer-scheduled event frame |
| Intensity | 0–100%; zero intensity is transparent |
| Wet / feedback | maximum 65% / 72% |
| Segment crossfade | 5–20 ms |
| RMS compensation | bounded 0.5–2× |
| Output guard | sample peak ceiling no higher than −0.3 dBFS |
| Repeatability | generator seed + variation + trigger index |

Event mode consumes scheduled frames emitted by the procedural renderer. It does not detect transients in arbitrary imported WAV files. This is offline signal processing, not a real-time callback or plugin hot-swap.

### State lifetime and storage

Working audio buffers and current UI state exist in the application process. Output WAV files and JSON manifests are written to the local output location selected by the user; existing output files are not overwritten. Closing the app ends the in-memory session but does not delete exported files.

The desktop UI stores its own preferences through the native preference layer. There is no browser localStorage, sessionStorage, IndexedDB, cookie-based state, shared server database or automatic cloud sync. There is no built-in latency/HTTP-500 simulator or external-service fallback because the audio path is offline.

## Configuration, environment and trust

There is no root `.env.example`, Docker Compose stack, account system, demo credentials, API token, CORS configuration, reverse proxy or local HTTPS setup. Audio generation and processing do not require network access.

Build options declared by `CMakeLists.txt`:

| CMake option | Default | Purpose |
|---|---:|---|
| `VENTU_BUILD_DESKTOP` | `ON` | look for Qt 6 Widgets and build the desktop target when available |
| `VENTU_BUILD_TESTS` | `ON` | build native tests |
| `VENTU_ENABLE_SANITIZERS` | `OFF` | enable AddressSanitizer/UndefinedBehaviorSanitizer on supported GCC/Clang builds |
| `REAPER_SOURCE_DIR` | empty | explicit local Reaper source checkout; enables the optional C17 expression bridge |

The Reaper bridge is disabled unless an explicit local source directory is provided. It uses the Reaper native expression ABI for deterministic integer control expressions; it does not replace the C++20 engine and is not inserted into the real-time DSP path.

### Plugin security boundary

The Plugin Manager has 19 built-in internal tools. A local `plugin.json` can be discovered recursively as metadata, but a discovered entry remains disabled until an explicitly reviewed native handler is attached. The manager does not load manifest entrypoints, arbitrary DLL/SO files or executable payloads. The public manifest schema is [ivan-vhs.plugin.v1](schemas/ivan-vhs.plugin.v1.schema.json).

Do not treat manifest discovery as code execution or a sandbox guarantee. Handler registration is an explicit native C++ operation reviewed by the application.

## Architecture and repository map

The data path is: **parameters + seed → procedural renderer → stereo float buffer → optional offline routing/DSP → PCM16 WAV + versioned JSON manifest**.

| Path | Contents |
|---|---|
| `src/` | C++20 generator, renderer, audio buffers, DSP kernels, plugin manager, Qt desktop UI and optional Reaper bridge |
| `tests/` | native DSP, generator, manifest, UI, plugin lifecycle and regression tests |
| `docs/` | architecture, features, plugin SDK, UI behavior and audio contracts |
| `schemas/` | versioned sample-pack and plugin manifest schemas |
| `tools/` | project-contract, fixture, visual-canon and engineering validators |
| `federation/` | explicit interoperability and adapter manifests |
| `assets/screenshots/` | captured desktop application image |
| `.github/workflows/` | Python contract, native, Linux Qt and Windows Qt CI jobs |
| `CMakeLists.txt` | C++20 build graph and optional feature switches |
| `project-contract.json` | machine-readable project contract |

The desktop uses Qt 6 Widgets. The generator and DSP engine are C++20. Python in CI validates repository contracts; it is not a Python web backend. There is no frontend/backend deployment, web server, mock server, browser bundle, port binding, CORS or public live preview. Browser support and responsive viewport rules are not applicable.

## CI, artifacts and operations

The [CI workflow](.github/workflows/ci.yml) includes:

- Python contract checks on Python 3.10, 3.11 and 3.12;
- a native C++20 core build and CTest run without the desktop target;
- Linux DSP + Qt desktop build on Ubuntu 24.04 with Qt 6 Widgets;
- Windows 11 native Qt desktop build on a Windows 2022 runner using Qt 6.8.3 and MSVC 2022;
- project-contract, generated-fixture, engineering-evolution and CLife validation.

The Windows desktop job captures the running app and uploads a screenshot artifact with 30-day retention. This workflow does **not** declare a public web deployment, publish a hosted preview, or create a tagged Windows release archive. Do not infer release packaging or automatic deployment from a passing CI job.

To roll back a local change, return to the previous Git commit or rebuild the last known-good revision. There is no running web service or deployed database to roll back. CI logs and artifacts are available from GitHub Actions.

## Troubleshooting

| Symptom | Check |
|---|---|
| Qt desktop target is missing | install Qt 6 Widgets or configure with `-DVENTU_BUILD_DESKTOP=OFF` for a core-only build |
| CMake version is rejected | install CMake 3.21 or newer |
| C++ compilation fails | confirm a C++20 compiler and inspect the first compiler diagnostic |
| WAV import is rejected | check PCM16, mono/stereo, 8–192 kHz, 128 MiB and 20M-frame source limits |
| Exported output differs | confirm the generator seed, variation and settings; stochastic routing must use the same options to repeat its route |
| Router changes imported audio unexpectedly | onset routing uses renderer-scheduled events; it does not transient-detect arbitrary imported WAV |
| Plugin manifest is visible but cannot run | discovery is metadata-only; attach an explicitly reviewed native handler |
| Reaper bridge is unavailable | set `REAPER_SOURCE_DIR` to an existing trusted Reaper source checkout at configure time |

For test or build failures, retain the exact CMake command, compiler/Qt versions, failing test name and full GitHub Actions log.

## Known limitations, contribution and license

VentuSample is an offline sample workstation, not a DAW or real-time audio host. The sample generator produces PCM16 stereo WAV and manifests; it does not contact external APIs, stream audio to a browser or manage cloud projects. The stochastic router uses eight built-in processors and does not host third-party plugins. Quantum Audio Engine is a creative DSP catalogue; Atomic-Quantum is a separate classical simulation layer, not physical quantum computation. Importing or exporting audio requires appropriate rights to the source material; a hash can establish byte identity but does not establish ownership.

No separate `CONTRIBUTING.md` was found. Propose improvements through [GitHub Issues](https://github.com/Ivan-VHS/VentuSample/issues) or a Pull Request. Include OS, compiler/CMake/Qt versions, the seed and exact settings, a safe input WAV description, and the exact command/output. Run CTest and the project-contract checks before submitting.

License: [Apache-2.0](LICENSE).

## Mapping to the 66 README rules

The web-stand checklist is documented against the actual offline desktop/audio product; web-only sections are explicitly marked not applicable:

| Rules | README coverage |
|---|---|
| 1–11 · purpose and boundaries | explains deterministic sample-workstation purpose, target users, implemented workflows, offline scope, feature and execution limits |
| 12–22 · quick start | gives CMake/C++20/Qt requirements, Linux/Windows/core-only commands, build and CTest checks |
| 23–33 · configuration and access | lists real CMake options and optional Reaper bridge; no fake ENV variables, accounts, secrets, CORS or HTTPS |
| 34–44 · data and behavior | describes seed-based repeatability, local WAV input/output, manifests, file limits, plugin discovery and UI state lifetime |
| 45–55 · architecture and build | maps real C++20/Qt architecture, DSP stages, repository paths, schemas and CI matrix |
| 56–66 · operations and support | links the real CI workflow, explains screenshot-artifact limits and rollback, troubleshooting, known issues, contribution path and license |

**There is no public VentuSample web-stand URL.** The desktop screenshot and CI screenshot artifact are not an interactive browser demo.
