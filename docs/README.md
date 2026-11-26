# VentuSample documentation

**Documentation baseline:** 2026-11-26 · **Project version:** 0.2.0 · **Runtime:** offline C++20 audio core.

Start with the [root README](../README.md) for build commands, supported file formats, size limits and offline-only behavior. The [Information Architecture](INFORMATION_ARCHITECTURE.md) page catalogs the complete library.

## Core technical references

- [Feature matrix](FEATURES.md) — implemented capabilities and unsupported workflows.
- [Architecture and audio contract](ARCHITECTURE.md) — renderer, routing, file bounds and manifest behavior.
- [Plugins](PLUGINS.md) — plugin registration, lifecycle and parameter boundaries.
- [Quantum Audio Engine](QUANTUM_AUDIO_ENGINE.md) — module catalogue and reusable kernels.
- [MIDI and resampling](MIDI_AND_RESAMPLING.md) — imported material and pitch behavior.
- [Quality Monolith Rhythm](QUALITY_MONOLITH_RHYTHM.md) — generator cohesion and regression parameters.
- [Reaper pack audit](REAPER_PACK_AUDIT.md) — metadata-only integration.
- [User guide](USER_GUIDE_RU.md) — workstation workflow.
- [Documentation control](DOCUMENTATION_CONTROL.md) — publication and evidence gates.

## Browse by topic

- [Audio pipeline and DSP](clusters/audio-dsp.md)
- [Studio, presets and plugin workflows](clusters/studio-workflows.md)
- [Architecture and runtime contracts](clusters/architecture-runtime.md)
- [UI and visual system](clusters/ui-visual.md)
- [Quality and engineering evidence](clusters/quality-engineering.md)
- [Integration, governance and planning](clusters/integration-governance.md)

## Shared standards

- [Unified documentation standard](UNIFIED_DOCUMENTATION_STANDARD.md)
- [Engineering baseline](ENGINEERING_BASELINE.md)
- [Contributing](../CONTRIBUTING.md)
- [Security](../SECURITY.md)

## Evidence and rights

Distinguish deterministic generation from objective audio quality, output peak/RMS metrics from subjective sound, and a manifest audit from waveform verification. Imported audio remains untrusted and subject to input bounds. Users remain responsible for rights to samples and generated packs.
