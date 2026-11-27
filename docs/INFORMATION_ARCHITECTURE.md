# Documentation information architecture

[Project README](../README.md) · [Documentation index](README.md)

This is the canonical navigation map for VentuSample. The root README is the entry point, this page is the documentation pillar, and topic hubs connect the audio/DSP implementation contracts with Studio workflows, architecture, UI and project integration.

## Navigation model

- **Entry point:** the root README explains the workstation, supported workflows and local operating boundaries.
- **Pillar:** this map organizes the repository's Markdown documentation into topic clusters and catalogs each page.
- **Topic hubs:** each hub groups related specifications and provides contextual routes between them.
- **Authority:** source, tests, manifests and schemas remain authoritative for audio behavior, supported formats, limits and maturity.

## Topic hubs

- [Audio pipeline and DSP](clusters/audio-dsp.md) — Audio rendering stages, numerical processing, MIDI, resampling, spectral analysis and audio-format contracts.
- [Studio, presets and plugin workflows](clusters/studio-workflows.md) — Generator and workstation capabilities, preset lifecycle, plugin interfaces and user workflows.
- [Architecture and runtime contracts](clusters/architecture-runtime.md) — Application boundaries, typed-value architecture, runtime integration and system-level contracts.
- [UI, accessibility and visual system](clusters/ui-visual.md) — Desktop interaction, accessible motion, visual assets and documentation rendering.
- [Quality, validation and evidence](clusters/quality-engineering.md) — Repeatability, engineering gates, numerical limits, validation and trustworthy status reporting.
- [Integration, governance and planning](clusters/integration-governance.md) — Project federation, replacement tools, roadmaps, ownership and supporting reference material.

## Cross-repository boundaries

The connected projects remain separate products. A link indicates useful context or an integration point, not shared ownership of audio content or automatic runtime compatibility.

- [Reaper documentation map](https://github.com/Ivan-VHS/Reaper/blob/main/docs/INFORMATION_ARCHITECTURE.md) — language, Studio and runtime integration contracts.
- [Pytyliki documentation map](https://github.com/Ivan-VHS/Pytyliki/blob/main/docs/INFORMATION_ARCHITECTURE.md) — processor systems and DSP migration context.
- [OPlague documentation map](https://github.com/Ivan-VHS/OPlague/blob/main/docs/INFORMATION_ARCHITECTURE.md) — device and driver inspection boundaries.
- [DAK documentation map](https://github.com/Ivan-VHS/DAK/blob/main/docs/INFORMATION_ARCHITECTURE.md) — repository entry-point map.

## Full document catalog

Each Markdown page has a direct catalog entry and a topic-hub entry. The cluster label supplies context; the page itself remains the source for its detailed technical claims.

### Audio pipeline and DSP

- [ATOMIC QUANTUM 133](ATOMIC_QUANTUM_133.md)
- [MIDI AND RESAMPLING](MIDI_AND_RESAMPLING.md)
- [OCTA8](OCTA8.md)
- [OFFLINE SPECTRUM ANALYSIS](OFFLINE_SPECTRUM_ANALYSIS.md)
- [PYTYLIKI DSP MIGRATION](PYTYLIKI_DSP_MIGRATION.md)
- [QUALITY MONOLITH RHYTHM](QUALITY_MONOLITH_RHYTHM.md)
- [QUANTUM AUDIO ENGINE](QUANTUM_AUDIO_ENGINE.md)
- [REAPER PACK AUDIT](REAPER_PACK_AUDIT.md)

### Studio, presets and plugin workflows

- [CRATE PRODUCTION SUITE](CRATE_PRODUCTION_SUITE.md)
- [CRATE PRODUCTION SUITE RU](CRATE_PRODUCTION_SUITE_RU.md)
- [HIP-HOP PRESET CRATE](../examples/hip-hop-presets/README.md)
- [CARE GUARD V1](CARE_GUARD_V1.md)
- [CLIFE](CLIFE.md)
- [FEATURES](FEATURES.md)
- [PLUGIN FEDERATION](PLUGIN_FEDERATION.md)
- [PLUGINS](PLUGINS.md)
- [PROGRAM CATALOG 1000](PROGRAM_CATALOG_1000.md)
- [SOURCE INTEGRATION](SOURCE_INTEGRATION.md)
- [USER GUIDE RU](USER_GUIDE_RU.md)

### Architecture and runtime contracts

- [ANATIDAE DESKTOP DESIGN SYSTEM](ANATIDAE_DESKTOP_DESIGN_SYSTEM.md)
- [ARCHITECTURE](ARCHITECTURE.md)
- [ENGINEERING BASELINE](ENGINEERING_BASELINE.md)
- [MOTION CONSTITUTION V2026](MOTION_CONSTITUTION_V2026.md)
- [TRANSACTIONAL RUNTIME](TRANSACTIONAL_RUNTIME.md)
- [RESILIENCE POLICY](RESILIENCE_POLICY.md)

### UI, accessibility and visual system

- [README VISUAL STANDARD](README_VISUAL_STANDARD.md)
- [UI MOTION](UI_MOTION.md)
- [UI UX 2026 CONVERGENCE](UI_UX_2026_CONVERGENCE.md)
- [UI UX CODEX 40026](UI_UX_CODEX_40026.md)
- [UI UX SYSTEM](UI_UX_SYSTEM.md)

### Quality, validation and evidence

- [DOCUMENTATION CONTROL](DOCUMENTATION_CONTROL.md)
- [ENGINEERING EVOLUTION FRAMEWORK](ENGINEERING_EVOLUTION_FRAMEWORK.md)
- [ENGINEERING](ENGINEERING.md)
- [IMPROVEMENT CONFLICT STANDARD](IMPROVEMENT_CONFLICT_STANDARD.md)
- [IVAN VHS ENGINEERING PROFILE](IVAN_VHS_ENGINEERING_PROFILE.md)
- [LANGUAGE QUALITY](LANGUAGE_QUALITY.md)
- [PROJECT STEWARDSHIP STANDARD](PROJECT_STEWARDSHIP_STANDARD.md)
- [QUALITY COMMERCIAL STANDARD](QUALITY_COMMERCIAL_STANDARD.md)
- [QUALITY REMEDIATION STANDARD](QUALITY_REMEDIATION_STANDARD.md)
- [QUALITY REPEATABILITY NATIVE](QUALITY_REPEATABILITY_NATIVE.md)
- [ROADMAP GOVERNANCE AND CI SCALE](ROADMAP_GOVERNANCE_CI_SCALE.md)
- [REPORT ENVELOPE](REPORT_ENVELOPE.md)

### Integration, governance and planning

- [CONTRIBUTING](../CONTRIBUTING.md)
- [VNT A0107](audio-errors/VNT-A0107.md)
- [VNT A0108](audio-errors/VNT-A0108.md)
- [VNT A0109](audio-errors/VNT-A0109.md)
- [AUTOMATION ROADMAP](AUTOMATION_ROADMAP.md)
- [DUCK SOVEREIGN CANON](DUCK_SOVEREIGN_CANON.md)
- [DUCKSTORY](DAKSTORY.md)
- [COCKOS REAPER](integrations/COCKOS_REAPER.md)
- [MARKET DEVELOPMENT STANDARD](MARKET_DEVELOPMENT_STANDARD.md)
- [MICROBUDGET PLANNER](MICROBUDGET_PLANNER.md)
- [REAPER RUNTIME INTEGRATION](REAPER_RUNTIME_INTEGRATION.md)
- [REPLACEMENT TOOLS](REPLACEMENT_TOOLS.md)
- [RESOURCE ALLOCATION 500 UAH](RESOURCE_ALLOCATION_500_UAH.md)
- [UNIFIED DOCUMENTATION STANDARD](UNIFIED_DOCUMENTATION_STANDARD.md)
- [ANATIDAE NAMING CANON](../federation/ANATIDAE_NAMING_CANON.md)
- [README](../README.md)
- [README.Ru](../README.ru.md)
- [README.Uk](../README.uk.md)
- [README.Zh CN](../README.zh-CN.md)
- [SECURITY](../SECURITY.md)

## Maintenance contract

New pages must be added to the appropriate topic hub and this catalog in the same change, with at least two useful inbound paths. Keep important pages within three to four clicks of the root README. Use direct relative links within this repository and canonical HTTPS links across repositories. Do not duplicate competing anchors or imply audio ownership, format compatibility or shipped capabilities without evidence.
