# AETHER-ANAS STUDIO // CINEFORGE QUANTUM
## Product constitution and implementation status

This document consolidates the AETHER-ANAS visual direction, CINEFORGE sample-family workflow, Ventu Audio Suite DSP rack and deterministic stochastic routing into one native desktop product direction.

## 1. Product boundary

VentuSample remains a native C++20 + Qt 6 desktop application. The application is offline-first and does not require a cloud account, network connection, analytics SDK, remote fonts or telemetry for local generation and processing.

The product is currently an offline renderer and workstation. It is not yet a low-latency real-time DAW, a VST3/AU host, a full WASM sandbox, a neural text-to-audio system or an out-of-process plugin sandbox. Built-in and reviewed SDK handlers run in the host process. A crashed process or memory-corrupting native handler is not isolated by the current plugin SDK.

## 2. Runtime topology

Qt Desktop → Project/Parameter Model → Generator + CINEFORGE + Audio Suite + Stochastic Router → Stereo Float DSP → Safety Pass → WAV + JSON Manifest

- Generator: seeded hip-hop loops, melodies, drums, bass and one-shot kits.
- CINEFORGE: scene-labelled procedural families, Dry/Wet, Attack/Body/Tail, combined preview and cineforge.pack.v1.
- Audio Suite: 66 deterministic offline stereo DSP processors in six functional families.
- Stochastic Router: seeded Micro-Variation, Round-Robin and Chaos One-Shot plans, optional parallel split, pinned slots and output guard.
- Plugin Manager: 18 first-party workspace/diagnostic tools and a reviewed in-process C++ SDK; manifest discovery is metadata-only.

The product-level 199-module vision is a catalogue target, not a completed count. The current Audio Suite contains 66 active DSP processors. The additional 133 scientific and speculative processor concepts remain future implementation candidates and must not be shown as executable modules until they have DSP code, UI, tests and documented signal semantics.

## 3. AETHER-ANAS visual canon

The UI uses a white/blue/red semantic system:
- White and feather white: #FFFFFF, #F4F6FA.
- Deep river / studio blue: #071124, #0A1E46, structural blue #0039A6, #0B1F3A.
- Signal red: #D52B1E, #B51A10; reserve red for Generate/Mutate, clipping and critical warnings.

Panels remain rectangular with 2–4 px corner radii. Use tabular numerals for BPM, key, sample rate, frame count, time and dB. Preserve keyboard focus indication and WCAG contrast. The interface prioritizes clear state, useful visualizers and progressive disclosure over decoration.

The existing Qt application already has light/dark themes and the shared color canon. New CINEFORGE and stochastic panels use the same palette. Scientific names such as Bloch sphere, quark confinement and gluon field are visualization metaphors for concrete DSP state; they do not imply quantum hardware or physical subatomic audio processing.

## 4. Audio constitution

### 4.1 Current operational modes

- Harlem / East Coast / Boom Bap generation: deterministic seeded patterns with editable tempo, swing, density and source-chop controls.
- Noir and cinematic design: CINEFORGE sound families and the 66-effect rack can be chained for offline sound design.
- Scene-to-Pack: the scene prompt is preserved in metadata while the current renderer uses explicit procedural family models and user controls.

### 4.2 CINEFORGE pack contract

Each pack has 8–12 assets, shared BPM and key metadata, a seed, a scene prompt, WAV export configuration, preview and JSON manifest. Each asset exports:
- Dry.wav — unprocessed family render.
- Wet.wav — deterministic multi-tap stereo reflection render.
- Attack.wav — transient layer generated at synthesis time.
- Body.wav — tonal/body layer generated at synthesis time.
- Tail.wav — atmosphere/decay layer generated at synthesis time.

Formats are stereo PCM 24-bit or IEEE Float 32-bit at 48/96 kHz. Output naming includes BPM, key, family, asset index and layer. The pack folder is deterministic for a seed; existing folders are never overwritten. The current Attack/Body/Tail outputs are synthesis stems, not machine-learned source separation.

### 4.3 Stochastic routing

The deterministic meta-host supports:
- Micro-Variation: preserve the selected chain and mutate only unlocked values inside a bounded drift.
- Round-Robin: precompute 4–8 separate candidate chains from derived seeds.
- Chaos One-Shot: create a new chain from the chosen pool using one seed.
- Freeze/Lock: preserve a selected slot's processor, position and parameter values.
- Parallel Split: render 2–4 independent branches and sum them with bounded gain.

The offline safety pass provides bounded RMS gain matching, DC blocking and a sample-peak ceiling. The default is −0.3 dBFS. This is not a standards-compliant oversampled true-peak limiter. PDC, live graph swaps, feedback routing and zero-click real-time crossfades are not claimed until an audio callback engine exists.

## 5. Quality invariants

1. A fixed seed and settings produce repeatable output.
2. Source files are never overwritten by processors.
3. WAV writers validate frame count, sample rate, encoding and RIFF size limits.
4. Non-finite samples are rejected or replaced before export.
5. DSP chains commit output only when every processor succeeds.
6. Random routing is bounded and leaves caller audio unchanged on failure.
7. Output filenames and manifests describe actual render settings.
8. No module appears as implemented without compiled code and regression tests.
9. Worker threads perform expensive offline rendering; UI objects are updated on the Qt event thread.
10. Claims of real-time latency, full sandboxing, neural inference or physical quantum modelling require separate verified implementations.

## 6. Current status and roadmap

| Capability | Status |
|---|---|
| Native offline sample generator | Implemented |
| AETHER-ANAS white/blue/red desktop direction | Implemented in shared Qt theme |
| 18 internal tools / PluginManager | Implemented |
| 66 DSP processors | Existing Audio Suite implementation; branch build validation required |
| Stochastic seeded routing modes | Added in this branch; CI validation required |
| CINEFORGE procedural pack generator | Added in this branch; CI validation required |
| Dry/Wet + Attack/Body/Tail exports | Added in this branch; CI validation required |
| 24-bit PCM / 32-bit float at 48/96 kHz | Added in this branch; CI validation required |
| 199 active processors | Not implemented; 133 additional concepts remain future work |
| Real-time audio callback and latency guarantee | Future milestone |
| PDC and glitch-free graph replacement | Future real-time engine milestone |
| Out-of-process DSP worker / Wasm sandbox | Future security/hosting milestone |
| VST3/AU hosting and DAW drag/drop | Future interoperability milestone |
| Neural prompt-to-audio and source separation | Future optional model milestone |

## 7. Build

~~~sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DVENTU_BUILD_DESKTOP=ON -DVENTU_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
~~~
