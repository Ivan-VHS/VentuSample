> **Navigation:** Start with the [information architecture map](INFORMATION_ARCHITECTURE.md), then choose the relevant [topic hub](clusters/audio-dsp.md). The map links the audio/DSP, Studio workflow, UI, quality and integration documentation.

## Topic hubs

- [Audio pipeline and DSP](clusters/audio-dsp.md) — Audio rendering stages, numerical processing, MIDI, resampling, spectral analysis and audio-format contracts.
- [Studio, presets and plugin workflows](clusters/studio-workflows.md) — Generator and workstation capabilities, preset lifecycle, plugin interfaces and user workflows.
- [Architecture and runtime contracts](clusters/architecture-runtime.md) — Application boundaries, typed-value architecture, runtime integration and system-level contracts.
- [UI, accessibility and visual system](clusters/ui-visual.md) — Desktop interaction, accessible motion, visual assets and documentation rendering.
- [Quality, validation and evidence](clusters/quality-engineering.md) — Repeatability, engineering gates, numerical limits, validation and trustworthy status reporting.
- [Integration, governance and planning](clusters/integration-governance.md) — Project federation, replacement tools, roadmaps, ownership and supporting reference material.

---

# Documentation

## Related OS product

[DuckOS — Mora application suite](https://github.com/Ivan-VHS/DuckOS/blob/docs/unified-documentation-standard/docs/PREINSTALLED_APPLICATIONS.md) documents the seven applications provided by the OS layer. This is a related product reference only; it does not add app-suite capabilities to VentuSample.

## Normative documentation standard

[Unified documentation and semantics standard](UNIFIED_DOCUMENTATION_STANDARD.md) defines the shared terminology, capability maturity, runtime states, verification outcomes, evidence rules, localization requirements and documentation acceptance criteria. It is normative for this repository; project-specific architecture and implementation evidence remain authoritative for the product itself.

| File | Purpose |
|---|---|
| [OCTA8.md](OCTA8.md) | OCTA-8 native C++20 typed-value contract and adapter |
| [FEATURES.md](FEATURES.md) | generator, pack, plugin and DSP capability matrix |
| [MIDI_AND_RESAMPLING.md](MIDI_AND_RESAMPLING.md) | bounded MIDI parser and offline sample-rate conversion |
| [OFFLINE_SPECTRUM_ANALYSIS.md](OFFLINE_SPECTRUM_ANALYSIS.md) | bounded C11 FFT and spectral descriptors |
| [PROGRAM_CATALOG_1000.md](PROGRAM_CATALOG_1000.md) | sound-project adoption map for the 1,000-item catalogue |
| [ARCHITECTURE.md](ARCHITECTURE.md) | audio pipeline, renderer and parameter boundaries |
| [PLUGINS.md](PLUGINS.md) | built-in tools, C++20 plugin SDK, metadata-only manifest discovery and lifecycle |
| [QUANTUM_AUDIO_ENGINE.md](QUANTUM_AUDIO_ENGINE.md) | 133 DSP module catalogue, 21 reusable kernels and offline WAV contract |
| [USER_GUIDE_RU.md](USER_GUIDE_RU.md) | operation, profile selection and Russian UI labels |
| [QUALITY_MONOLITH_RHYTHM.md](QUALITY_MONOLITH_RHYTHM.md) | 20×3 quality/cohesion/rhythm acceptance matrix |
| [SOURCE_INTEGRATION.md](SOURCE_INTEGRATION.md) | WAV constraints and source-rights requirements |
| [REAPER_PACK_AUDIT.md](REAPER_PACK_AUDIT.md) | manifest-only interoperability with Reaper |
| [UI_MOTION.md](UI_MOTION.md) | persisted Qt motion preferences, project-BPM indicator and accessibility limits |
| [ANATIDAE_DESKTOP_DESIGN_SYSTEM.md](ANATIDAE_DESKTOP_DESIGN_SYSTEM.md) | visual tokens and workbench UX |
| [PROJECT_STEWARDSHIP_STANDARD.md](PROJECT_STEWARDSHIP_STANDARD.md) | release, privacy and communication rules |
| [schemas/ventusample.pack.v1.schema.json](../schemas/ventusample.pack.v1.schema.json) | pack schema v1 |
| [schemas/ventusample.pack.v2.schema.json](../schemas/ventusample.pack.v2.schema.json) | tonal metadata schema v2 |
| [../schemas/ivan-vhs.plugin.v1.schema.json](../schemas/ivan-vhs.plugin.v1.schema.json) | shared plugin manifest schema |

The root README contains the live Windows desktop capture and current build commands. Source and tests define actual behavior; a manifest digest does not prove sample ownership. Plugin discovery is metadata-only, and DSP processing is offline.

## Publishing standard

[README and visual documentation standard](README_VISUAL_STANDARD.md) — information architecture, Markdown/MDX, diagrams, screenshots, image formats, accessibility, localization and quality gates.


## Motion and embedded graphics

[Motion Constitution v2026](MOTION_CONSTITUTION_V2026.md) is the shared source for timing/easing tokens, adaptive motion, reduced-motion precedence, themes, the 2×2/focus-scene lab layout, SVG/Canvas telemetry rules and the four-domain capability matrix. [UI_MOTION.md](UI_MOTION.md) records the native behavior implemented in this repository; planned showcase features are not reported as shipped.


## Bounded 500 UAH allocation reference

- [Multilingual specification](RESOURCE_ALLOCATION_500_UAH.md) — bounded budget model, scenarios and operating rules.
- [Offline interactive simulator](resource-allocation-500-uah.html) — adjust category amounts and see the remainder or overrun.
- [Example JSON](../examples/resource-allocation-500-uah.json) and [JSON Schema](../schemas/resource-allocation.v1.schema.json).

This is an illustrative personal-budgeting reference, not a claim that the host project manages payments or live retail prices.


## Language quality

[Language and editorial quality](LANGUAGE_QUALITY.md) — orthography, punctuation, lexical precision, syntax, multilingual review and conservative prose checks.


## Project-specific engineering profile

[IVAN_VHS_ENGINEERING_PROFILE.md](IVAN_VHS_ENGINEERING_PROFILE.md) translates the project's direction into concrete architecture boundaries, implementation priorities, human-readable code conventions and acceptance tests. It is a development contract; it does not mark planned work as implemented.
