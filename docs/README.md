# VentuSample documentation

Start with the [project README](../README.md) for the product overview, build steps and current limits. The [documentation map](INFORMATION_ARCHITECTURE.md) links the full catalogue.

## Key pages
- [NEXUS system-service boundaries](NEXUS_SYSTEM_SERVICE_BOUNDARIES_V1.md) — AudioSubsys ownership and offline-processing limits.

- [Features and maturity](FEATURES.md)
- [Architecture](ARCHITECTURE.md)
- [Audio engine](QUANTUM_AUDIO_ENGINE.md)
- [MIDI and resampling](MIDI_AND_RESAMPLING.md)
- [Plugin interfaces](PLUGINS.md)
- [NEXUS audio profile](NEXUS_AUDIO_PROFILE_V1.md) — offline processing ownership and companion-tool integration.
- [Source and licensing rules](SOURCE_INTEGRATION.md)

## Browse by topic

- [Audio pipeline and DSP](clusters/audio-dsp.md)
- [Studio and plugin workflows](clusters/studio-workflows.md)
- [Architecture and runtime](clusters/architecture-runtime.md)
- [UI and accessibility](clusters/ui-visual.md)
- [Quality and evidence](clusters/quality-engineering.md)
- [Integration and governance](clusters/integration-governance.md)

## Shared rules

- [Documentation standard](UNIFIED_DOCUMENTATION_STANDARD.md)
- [Engineering evolution framework](ENGINEERING_EVOLUTION_FRAMEWORK.md)
- [Repository evolution directives](REPOSITORY_EVOLUTION_DIRECTIVES.md)

DSP processing is offline. Source ownership, format limits and implemented behavior must be checked against the relevant contracts, source and tests.
