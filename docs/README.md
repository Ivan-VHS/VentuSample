# VentuSample documentation

The [root README](../README.md) is the public entry point for project purpose, current maturity, build requirements, quick start, screenshots and implementation boundaries. This index links to the deeper technical contracts.

## Topic hubs

| Hub | Scope |
|---|---|
| [Audio pipeline and DSP](clusters/audio-dsp.md) | Rendering, numerical processing, MIDI, resampling, spectrum and audio-format contracts |
| [Studio, presets and plugin workflows](clusters/studio-workflows.md) | Generator, Qt workstation, preset lifecycle, plugin interfaces and operator workflows |
| [Architecture and runtime contracts](clusters/architecture-runtime.md) | Application layers, typed-value model, offline processing and system integration |
| [UI, accessibility and visual system](clusters/ui-visual.md) | Native desktop interaction, keyboard use, reduced motion, theme and assets |
| [Quality, validation and evidence](clusters/quality-engineering.md) | Determinism, numerical bounds, test coverage and trustworthy status reporting |
| [Integration, governance and planning](clusters/integration-governance.md) | Reaper bridge, DuckOS federation, ownership, roadmap and reference material |

## Core documents

| Document | Purpose |
|---|---|
| [Architecture](ARCHITECTURE.md) | Renderer, DSP stages, parameter flow and output contracts |
| [Features](FEATURES.md) | Capability inventory and maturity boundaries |
| [OCTA-8](OCTA8.md) | Native C++20 typed-value contract and adapter |
| [MIDI and resampling](MIDI_AND_RESAMPLING.md) | Bounded MIDI parser and offline sample-rate conversion |
| [Offline spectrum analysis](OFFLINE_SPECTRUM_ANALYSIS.md) | Bounded C11 FFT and spectral descriptors |
| [Quantum Audio Engine](QUANTUM_AUDIO_ENGINE.md) | Typed module catalogue and reusable offline kernels |
| [Plugins](PLUGINS.md) | Built-in tools, C++20 plugin contract and metadata-only discovery |
| [Source integration](SOURCE_INTEGRATION.md) | WAV import constraints and source-rights requirements |
| [Reaper pack audit](REAPER_PACK_AUDIT.md) | Manifest-only interoperability with Reaper |
| [Visual documentation standard](README_VISUAL_STANDARD.md) | README layout, screenshots, diagrams, accessibility and publication gates |
| [Contributing](../CONTRIBUTING.md) | Local build, tests and pull-request requirements |

The normative [Unified Documentation Standard](UNIFIED_DOCUMENTATION_STANDARD.md) defines maturity labels, evidence rules, terminology, localization and documentation acceptance criteria. The repository-specific implementation and tests remain authoritative for product behavior.

## Repository map

~~~text
src/                       C++20 generator, DSP integration and optional Qt desktop
src/compat/pytyliki-dsp/   C11 DSP support library
tests/                     generator, DSP, preset and contract tests
schemas/                   versioned pack, preset and plugin schemas
docs/clusters/             focused architecture and workflow hubs
assets/readme/             editable SVG visual assets
assets/screenshots/        real desktop captures
.github/workflows/         CI, native build and desktop capture
~~~

## Evidence and scope

Source code, tests and schemas define what the application currently does. A catalogue entry or manifest digest is not evidence of audio ownership, plugin trust or runtime availability. Plugin discovery remains metadata-only, generated/imported audio rights remain the operator's responsibility, and DSP processing is offline.
