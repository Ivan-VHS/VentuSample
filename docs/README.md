# Documentation

| File | Purpose |
|---|---|
| [FEATURES.md](FEATURES.md) | generator, pack, plugin and DSP capability matrix |
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
| [SECURITY_ASSURANCE_V2.md](SECURITY_ASSURANCE_V2.md) | federated source audit, WAV/DSP fuzz-smoke, dependency review, privacy tests and shared scoring |

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
