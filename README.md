# VentuSample — procedural sample workstation

[CI](https://github.com/Ivan-VHS/VentuSample/actions/workflows/ci.yml) · C++20 · Qt 6 Widgets · Windows/Linux · Offline · Apache-2.0

VentuSample is a native workstation for deterministic procedural hip-hop sample packs. Its scalar renderer exports PCM16 stereo WAV files and versioned JSON manifests without requiring a DAW, audio device or network connection.

## Live Windows desktop

<p align="center"><img src="assets/screenshots/ventusample-studio.png" alt="VentuSample running on Windows 11" width="100%"></p>
<sub>Actual 1920×1080 Windows 11 desktop capture from the Qt CI build. The screenshot shows the generator workbench; plugin discovery and the DSP laboratory are separate menu actions.</sub>

## UI motion and accessibility

VentuSample provides **View → Motion Settings** with persisted **Reduce Motion**, **Eco / Low-CPU Visuals** and **Sync to BPM** preferences. The internal project BPM drives a restrained beat indicator; this is project-tempo feedback, not an external DAW transport link. Reduce Motion and Eco suspend that indicator and cap retained transitions at 80 ms. Qt animations and the beat timer stay on the UI thread, outside DSP callbacks, audio buffers, generator workers and stochastic-routing code. See the [UI motion contract](docs/UI_MOTION.md).

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

The native PluginManager includes 19 built-in internal tools. The **Плагины → Менеджер плагинов** view supports inspection, enable/disable and bounded dispatch. A public C++20 SDK exposes reviewed handler registration, lazy initialization, shutdown callbacks and diagnostics.

Local `plugin.json` manifests can be discovered recursively, but discovery is metadata-only. A discovered entry remains disabled until a reviewed native handler is explicitly attached. Automatic DLL/SO loading, manifest-driven code execution, marketplace installation and sandbox claims are not supported. The public manifest contract is [ivan-vhs.plugin.v1](schemas/ivan-vhs.plugin.v1.schema.json). See [Plugin platform and SDK](docs/PLUGINS.md).

## Quantum Audio Engine — 133 DSP modules

Open **Плагины → Quantum Audio Engine — 133 DSP** for the separate offline audio laboratory. It contains 133 stable module identities across five families and routes them through 21 reusable, tested C++ DSP kernels. Load a supported WAV, assemble an ordered chain, adjust Amount / Tone / Rate / Wet-Dry, process offline and export stereo PCM16 WAV.

The catalogue is a creative signal-processing system, not physical quantum computation. It is not a real-time audio engine, DAW, VST/AU host or arbitrary native-library loader. See [Quantum Audio Engine](docs/QUANTUM_AUDIO_ENGINE.md) for input/output constraints, catalogue and DSP boundaries.

## Atomic-Quantum 133 simulation layer

A separate C++20 research library registers 133 atomic/quantum operation IDs and implements a bounded state-vector subset: state validation, normalization, Hadamard/phase/Pauli/CNOT/SWAP gates, Bell/GHZ preparation, seeded projective measurement, stochastic noise trajectories and pure-state fidelity. This is distinct from the existing **Quantum Audio Engine — 133 DSP modules** above: the audio catalogue remains intact and is not physical quantum computing. Advanced research functions remain catalogued until their numerical models are independently implemented and tested. See [Atomic-Quantum 133](docs/ATOMIC_QUANTUM_133.md).

## Stochastic routing

The optional **Stochastic Router** processes rendered audio buffers before PCM16 WAV export. Its weighted pool contains eight selectable processors: low-pass, high-pass, saturator, bitcrusher, delay, reverb, ring modulation and stereo width. It does not load, discover or instantiate arbitrary third-party plugins.

- **Topologies:** hybrid, serial or parallel; parallel routing uses 2–4 branches when enough effects are enabled.
- **Triggers:** one route per file, per bar, or per scheduled generator event. Event mode uses renderer-supplied event frames; imported WAV files are not separately transient-detected.
- **Controls:** intensity 0–100%; aggression, space, dirt and chaos follow the generator's grit, stereo, saturation and density controls.
- **Boundaries:** wet ≤65%, feedback ≤72%, segment fades 5–20 ms, RMS compensation 0.5–2×, and output sample peak guarded at ≤−0.3 dBFS.
- **Repeatability:** the generator seed, variation, topology, trigger and enabled-effect mask determine the route.

The generator option is off by default. When enabled, routing runs offline on the rendered float buffer before PCM16 encoding; manifest v2 records the settings and selected route names per output file. A Plugin Manager run performs a one-bar in-memory preview and does not write files.

This is an **offline DSP path**. It is not a real-time callback, playback hot-swap system, VST/AU host or arbitrary third-party audio-plugin loader.

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

## DUCK CORE // README & Motion Suite

The shared visual reference for interactive demos and UI documentation defines three palettes: **Pond Noir** (default dark canvas), **Golden Reed** (warm high-contrast accents) and **Arctic Glacier** (cool cyan/blue accents). Use semantic tokens for canvas, surfaces, borders, text, accents and focus states instead of unrelated hard-coded component colors.

Motion follows a clear hierarchy: restrained spring easing for intentional entrances and feedback, smooth easing for routine transitions, and optional Zen / Hyper intensity presets only where the application exposes them. Parallax, floating geometry and ripple effects are decorative, nonessential and must never carry information or block input. The `reduced-motion` preference overrides every preset and suppresses decorative movement.

**Accessibility and responsive behavior:** preserve readable contrast, visible keyboard focus, touch-sized controls, predictable active/disabled states and layouts that collapse cleanly on narrow screens. Motion must not cause layout shifts or make controls harder to operate.

This is a DUCK CORE visual reference, **not a claim** that every theme, animation or control ships in every application. The implementation and project-specific contract are authoritative: [UI motion contract](docs/UI_MOTION.md).


## Motion Constitution v2026

The shared interface contract is [Motion Constitution v2026](docs/MOTION_CONSTITUTION_V2026.md). It standardizes spring/snappy easing, the 0.4×–2.2× motion-scale envelope, reduced-motion precedence, light/dark parity, the 2×2/focus-scene showcase and data-bound SVG/Canvas telemetry. The [native Qt motion contract](docs/UI_MOTION.md) defines the current implementation: bounded UI-thread transitions, persisted motion preferences and the optional project-BPM indicator. VentuSample is an offline sample-generation/DSP workstation, not a real-time DAW or external mixer; meters and waveforms must be derived from actual preview/render buffers and must never run inside DSP callbacks or generator workers.

## Shared motion architecture map

<p align="center"><img src="assets/motion-constitution-v2026.svg" alt="Motion Constitution v2026: four-domain architecture map" width="100%"></p>

This is the shared 2×2 architecture reference, not a live application screenshot. It distinguishes shipped product scope from unavailable or planned capabilities.
