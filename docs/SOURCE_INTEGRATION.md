# Source-document and repository integration ledger

This file records how the supplied design document and the sibling repositories were translated into VentuSample. The integration rule is **reuse engineering contracts, not unrelated product behavior**.

## 1. Primary source document

- Library asset: `рипер язык (1).txt`
- Size: 1,378,616 bytes · 801,693 Unicode characters · 5,484 lines
- SHA-256: `1a644aea55eb87bc560e72077a37c5360d6658a63dc00ad823481f1a11b690d0`
- Source share ID: `libfile_42d7db1cce80819180219a8640e1a0e7`

The document contains a desktop-environment and language-architecture specification, a 12-commandment design section, a 66-law engineering manifesto, a 666-rule architecture compendium, and a 65/25/10 white-blue-red desktop UI excerpt. The source is not a dedicated music-production specification; its contribution is the engineering foundation below. The complete file remains in the user's Library and is referenced by digest rather than copied wholesale into the application.

### Applied to VentuSample

| Source idea | Application of the idea | Evidence in this repository |
|---|---|---|
| Hydrodynamic calm / low visual noise | Clean parameter groups, visible hierarchy, compact status and scrollable workspace | `src/mainwindow.cpp` |
| White / blue / red design system | Separate light/dark semantic palettes; red is reserved for card/tab emphasis and warning semantics | `src/mainwindow.cpp` |
| Human-first desktop ergonomics | Native file/folder dialogs, keyboard-operable controls, readable labels and non-blocking generation | `src/mainwindow.cpp` |
| Explicit state and deterministic execution | Typed `Parameters`, validated ranges and per-variation seeded PRNG | `src/generator.hpp`, `src/generator.cpp` |
| Bounded resource model | Maximum bars/variations, WAV byte/frame limits, finite-value checks and RIFF-size checks | `src/generator.cpp` |
| Ownership and deterministic release | RAII for file streams, vectors and UI objects; no hidden persistent audio buffers | `src/generator.cpp`, `src/mainwindow.cpp` |
| Data-flow clarity / modularity | Desktop, control plane, renderer, asset I/O and verification are separate layers | `docs/ARCHITECTURE.md` |
| Backpressure / fault isolation | One bounded pack job at a time; rendering runs off the UI thread and reports a single result/error | `src/mainwindow.cpp` |
| Transparency / audit | Export manifest records schema, musical parameters, source filename and per-file measurements | `src/generator.cpp` |
| Platform adaptation | Qt 6 desktop target with CMake and portable scalar renderer | `CMakeLists.txt` |
| Quality / cohesion / rhythm framework | Shared drum/melody swing grid, four-bar turnarounds and stereo-linked bus gain; one validated parameter snapshot remains the source of truth | `src/generator.cpp`, `tests/test_generator.cpp`, `docs/QUALITY_MONOLITH_RHYTHM.md` |
| Open ecosystem / contract versioning | Versioned pack schema and a separate federation map | `schemas/ventusample.pack.v1.schema.json`, `federation/octa-pyramid.v1.json` |

### Adaptation of the 66 laws and 666 rules

The 66 laws are applied as engineering invariants rather than copied as programming-language features. The physics/resource group becomes explicit memory/file bounds and visible processing cost; execution-flow laws become deterministic seeded rendering, explicit threading and no hidden state; mathematics/logic laws become strict parameter and WAV-contract validation; lifecycle laws become isolated failure handling; systems laws become modularity, reproducible builds and inspectable measurements.

The 666-rule compendium's six domains are treated as a completeness checklist:

1. **Execution media:** local CPU rendering is implemented; distributed/cloud/quantum/photonic execution is intentionally not applicable to this offline workstation.
2. **Calm surface and transparency:** responsive, inspectable UI and source-visible parameter mappings are implemented.
3. **Type/semantic discipline:** typed parameter structures and fail-fast range/format checks are implemented; a new programming-language type system is not applicable.
4. **Effects and safety boundary:** imported WAV is treated as untrusted, bounded data; no imported executable/native code is loaded.
5. **Concurrency and flow control:** a single worker-thread batch render avoids UI blocking; a distributed actor/NUMA runtime is not applicable.
6. **Human and ecosystem freedom:** the app has modular portable C++ code, versioned pack metadata and no cloud account or mandatory external sample service.

The document also contains unrelated programming-language, medical, proverb and research material. Those domain-specific claims are not represented as audio features and are not silently copied into the product.

## 2. Reaper — desktop and orchestration architecture

Reused concepts from the Reaper desktop design system, UI/UX architecture, project contract and OCTA-8 federation documents:

- White/blue/red semantic tokens, clear hierarchy, DPI-aware native desktop intent, readable type and reduced-motion-safe interaction.
- Independent product boundaries; versioned JSON at project boundaries; evidence-backed capability claims.
- An eight-facet federation model (identity, contracts, transport, provenance, validation, privacy, safety and audit).

VentuSample remains an independent audio product; it does not embed Reaper's parser, IR, runtime, compiler or language semantics. The exported pack contract is the integration point.

Reference: [Reaper repository](https://github.com/Ivan-VHS/Reaper).

## 3. Pytyliki — DSP and bounded-memory engineering

Reused concepts from `docs/PROCESSOR_NATIVE_DSP.md`, `docs/MEMORY_ARENA.md`, `docs/ARCHITECTURE.md` and the existing scalar DSP implementation:

- Separate control-plane validation from sample-buffer processing.
- A portable scalar reference implementation before optional acceleration.
- Explicit sample rate/channel/frame validation, fixed input bounds and per-render state.
- Offline PCM conversion, gain/filter/dynamics/measurement concepts.

VentuSample currently uses its own C++20 scalar audio engine and does not load a Pytyliki binary. A future C ABI cross-check must be explicitly selected, versioned and tested before promotion.

Reference: [Pytyliki repository](https://github.com/Ivan-VHS/Pytyliki).

## 4. OPlague — safety, persistence and evidence quality

Reused concepts from OPlague's desktop design, atomic settings persistence, project contract and unified engineering standard:

- Best-effort persistence of user preferences outside the source tree.
- Reject malformed, oversized or non-finite external data before processing.
- Keep optional integrations explicit; do not load native libraries implicitly.
- Distinguish verified output facts from planned or unsupported behavior.

OPlague's PnP inventory, driver-package inspection, device compatibility graph, UDRV format and privileged hardware operations are outside the audio product boundary and are deliberately not included.

Reference: [OPlague repository](https://github.com/Ivan-VHS/OPlague).

## 5. VentuSample capability truth table

| Capability | Status |
|---|---|
| Native Qt 6 desktop source and CMake target | Implemented; Qt-enabled build is CI-gated |
| Procedural hip-hop audio generation | Implemented in portable C++20 core |
| Seeded variations | Implemented and unit-tested |
| PCM16 WAV export and manifest | Implemented and unit-tested |
| PCM16 WAV import / four-way chop / pitch resampling | Implemented and unit-tested |
| Real-time playback / DAW plugin | Not implemented |
| Time-preserving pitch shift | Not implemented |
| GPU / SIMD dispatch | Not implemented; scalar baseline is authoritative |
| Optional Pytyliki native DSP bridge | Specified as a future explicit adapter; not loaded |
| Reaper pack consumer / round-trip recipe automation | Versioned file contract published; consumer automation not yet implemented |

This ledger prevents a specification, source document or sibling project's capability from being presented as a working VentuSample feature without implementation and test evidence.
