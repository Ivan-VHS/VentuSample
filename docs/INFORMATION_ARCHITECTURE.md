# VentuSample documentation information architecture

[Project README](../README.md) · [Documentation index](README.md)

This is the canonical map for the VentuSample documentation library. Its full catalog is derived from the repository tree. Source, tests, manifests and schemas define actual audio behavior, accepted formats and feature maturity.

## Entry points and authority

- **Project entry:** [README](../README.md) — offline workstation scope, build workflow and audio limits.
- **Documentation home:** [docs/README.md](README.md) — key technical pages and topic hubs.
- **Feature matrix:** [FEATURES](FEATURES.md) — current capability boundaries.
- **Architecture:** [ARCHITECTURE](ARCHITECTURE.md) — audio pipeline and file contracts.
- **Authority:** C++/C source defines DSP behavior; tests establish exercised inputs; schemas define pack contracts; CMake defines optional desktop behavior. Documentation must not claim unimplemented real-time or device features.

## Topic hubs

- [Audio pipeline and DSP](clusters/audio-dsp.md)
- [Studio, presets and plugin workflows](clusters/studio-workflows.md)
- [Architecture and runtime contracts](clusters/architecture-runtime.md)
- [UI, accessibility and visual system](clusters/ui-visual.md)
- [Quality, validation and evidence](clusters/quality-engineering.md)
- [Integration, governance and planning](clusters/integration-governance.md)

## Cross-repository boundaries

- [Reaper documentation map](https://github.com/Ivan-VHS/Reaper/blob/main/docs/INFORMATION_ARCHITECTURE.md)
- [Pytyliki documentation map](https://github.com/Ivan-VHS/Pytyliki/blob/main/docs/INFORMATION_ARCHITECTURE.md)
- [OPlague documentation map](https://github.com/Ivan-VHS/OPlague/blob/main/docs/INFORMATION_ARCHITECTURE.md)
- [DAK documentation map](https://github.com/Ivan-VHS/DAK/blob/main/docs/INFORMATION_ARCHITECTURE.md)

Pack exchange is a file-based interface. It does not imply shared rendering, shared ownership of audio material or a common runtime.

## Complete documentation catalog

Every maintained Markdown page under docs/ is listed below. A catalog entry provides navigation; it does not prove implementation maturity.

### A

- [ANATIDAE DESKTOP DESIGN SYSTEM](ANATIDAE_DESKTOP_DESIGN_SYSTEM.md)
- [ARCHITECTURE](ARCHITECTURE.md)
- [ATOMIC QUANTUM 133](ATOMIC_QUANTUM_133.md)
- [AUTOMATION ROADMAP](AUTOMATION_ROADMAP.md)

### C

- [CALLIGRAPHIC TYPOGRAPHY](CALLIGRAPHIC_TYPOGRAPHY.md)
- [CARE GUARD V1](CARE_GUARD_V1.md)
- [CLIFE](CLIFE.md)

### D

- [DEFENSIVE SECURITY DOCTRINE](DEFENSIVE_SECURITY_DOCTRINE.md)
- [DOCUMENTATION CONTROL](DOCUMENTATION_CONTROL.md)
- [DUCKSTORY](DUCKSTORY.md)
- [DUCK SOVEREIGN CANON](DUCK_SOVEREIGN_CANON.md)

### E

- [ENGINEERING](ENGINEERING.md)
- [ENGINEERING BASELINE](ENGINEERING_BASELINE.md)
- [ENGINEERING EVOLUTION FRAMEWORK](ENGINEERING_EVOLUTION_FRAMEWORK.md)

### F

- [FEATURES](FEATURES.md)

### I

- [IMPROVEMENT CONFLICT STANDARD](IMPROVEMENT_CONFLICT_STANDARD.md)
- [INTERNATIONALIZATION STANDARD](INTERNATIONALIZATION_STANDARD.md)
- [IVAN VHS ENGINEERING PROFILE](IVAN_VHS_ENGINEERING_PROFILE.md)

### K

- [KATYUHEN INTEGRATION](KATYUHEN_INTEGRATION.md)

### L

- [LANGUAGE QUALITY](LANGUAGE_QUALITY.md)

### M

- [MARKET DEVELOPMENT STANDARD](MARKET_DEVELOPMENT_STANDARD.md)
- [MICROBUDGET PLANNER](MICROBUDGET_PLANNER.md)
- [MIDI AND RESAMPLING](MIDI_AND_RESAMPLING.md)
- [MORA DESIGN SYSTEM](MORA_DESIGN_SYSTEM.md)
- [MORA DESKTOP INTEGRATION](MORA_DESKTOP_INTEGRATION.md)
- [MOTION CONSTITUTION V2026](MOTION_CONSTITUTION_V2026.md)

### N

- [NEXUS AUDIO PROFILE V1](NEXUS_AUDIO_PROFILE_V1.md)
- [NEXUS SYSTEM SERVICE BOUNDARIES V1](NEXUS_SYSTEM_SERVICE_BOUNDARIES_V1.md)

### O

- [OCTA8](OCTA8.md)
- [OFFLINE SPECTRUM ANALYSIS](OFFLINE_SPECTRUM_ANALYSIS.md)

### P

- [PLUGINS](PLUGINS.md)
- [PLUGIN FEDERATION](PLUGIN_FEDERATION.md)
- [PROGRAM CATALOG 1000](PROGRAM_CATALOG_1000.md)
- [PROJECT STEWARDSHIP STANDARD](PROJECT_STEWARDSHIP_STANDARD.md)
- [PYTYLIKI DSP MIGRATION](PYTYLIKI_DSP_MIGRATION.md)

### Q

- [QUALITY COMMERCIAL STANDARD](QUALITY_COMMERCIAL_STANDARD.md)
- [QUALITY MONOLITH RHYTHM](QUALITY_MONOLITH_RHYTHM.md)
- [QUALITY REMEDIATION STANDARD](QUALITY_REMEDIATION_STANDARD.md)
- [QUALITY REPEATABILITY NATIVE](QUALITY_REPEATABILITY_NATIVE.md)
- [QUANTUM AUDIO ENGINE](QUANTUM_AUDIO_ENGINE.md)

### R

- [README VISUAL STANDARD](README_VISUAL_STANDARD.md)
- [REAPER PACK AUDIT](REAPER_PACK_AUDIT.md)
- [REAPER RUNTIME INTEGRATION](REAPER_RUNTIME_INTEGRATION.md)
- [RELEASE](RELEASE.md)
- [REPLACEMENT TOOLS](REPLACEMENT_TOOLS.md)
- [REPORT ENVELOPE](REPORT_ENVELOPE.md)
- [REPOSITORY EVOLUTION DIRECTIVES](REPOSITORY_EVOLUTION_DIRECTIVES.md)
- [RESILIENCE POLICY](RESILIENCE_POLICY.md)
- [RESOURCE ALLOCATION 500 UAH](RESOURCE_ALLOCATION_500_UAH.md)
- [ROADMAP GOVERNANCE CI SCALE](ROADMAP_GOVERNANCE_CI_SCALE.md)

### S

- [SOURCE INTEGRATION](SOURCE_INTEGRATION.md)
- [SYSTEM CAPABILITY PROFILE](SYSTEM_CAPABILITY_PROFILE.md)

### T

- [TRANSACTIONAL RUNTIME](TRANSACTIONAL_RUNTIME.md)

### U

- [UI MOTION](UI_MOTION.md)
- [UI UX 2026 CONVERGENCE](UI_UX_2026_CONVERGENCE.md)
- [UI UX CODEX 40026](UI_UX_CODEX_40026.md)
- [UI UX SYSTEM](UI_UX_SYSTEM.md)
- [UNIFIED DOCUMENTATION STANDARD](UNIFIED_DOCUMENTATION_STANDARD.md)
- [USER GUIDE RU](USER_GUIDE_RU.md)

### Nested references

- [VNT A0107](audio-errors/VNT-A0107.md)
- [VNT A0108](audio-errors/VNT-A0108.md)
- [VNT A0109](audio-errors/VNT-A0109.md)
- [architecture runtime](clusters/architecture-runtime.md)
- [audio dsp](clusters/audio-dsp.md)
- [integration governance](clusters/integration-governance.md)
- [quality engineering](clusters/quality-engineering.md)
- [studio workflows](clusters/studio-workflows.md)
- [ui visual](clusters/ui-visual.md)
- [COCKOS REAPER](integrations/COCKOS_REAPER.md)

## Maintenance and review

- Keep the offline-rendering and no-audio-device boundary visible in user and architecture docs.
- Keep source WAV limits, accepted rates, output encoding and pack schema versions aligned with tests.
- Document changes to DSP topology, triggers, feedback/wet bounds and peak protection with regression evidence.
- Distinguish generator metadata from analysis of imported source audio.
- Update the map and topic hub when pages are added, moved or retired.
- Keep EN/RU/UK/ZH-CN README facts and section order synchronized.
- Preserve sample-rights notices, Apache-2.0 and third-party attribution.
- Verify all local links and build commands before release; use DAK's canonical 50-point README standard.
