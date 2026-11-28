# Audio pipeline and DSP

[Documentation home](../README.md) · [Information architecture](../INFORMATION_ARCHITECTURE.md)

Audio rendering stages, numerical processing, MIDI, resampling, spectral analysis and audio-format contracts.

This hub groups the related contracts for the audio workstation. Use each target document for its exact supported formats, numeric limits and capability maturity; a navigation relationship is not a compatibility guarantee.

## Documents

- [ATOMIC QUANTUM 133](../ATOMIC_QUANTUM_133.md)
- [MIDI AND RESAMPLING](../MIDI_AND_RESAMPLING.md)
- [OCTA8](../OCTA8.md)
- [OFFLINE SPECTRUM ANALYSIS](../OFFLINE_SPECTRUM_ANALYSIS.md)
- [PYTYLIKI DSP MIGRATION](../PYTYLIKI_DSP_MIGRATION.md)
- [QUALITY MONOLITH RHYTHM](../QUALITY_MONOLITH_RHYTHM.md)
- [QUANTUM AUDIO ENGINE](../QUANTUM_AUDIO_ENGINE.md)
- [CRATE PRODUCTION SUITE](../CRATE_PRODUCTION_SUITE.md) — sample chops, pocket timing, beat routing and WAV prints.
- [REAPER PACK AUDIT](../REAPER_PACK_AUDIT.md)

## Navigation rules

- Keep processing, file-format, plugin and user-workflow contracts distinct where their responsibilities differ.
- Cross-link documents when a user workflow or data contract connects them directly.
- Update this hub and the central catalog whenever a page is added, moved or retired.
