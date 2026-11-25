# VentuSample — NEXUS Audio Profile v1

**Owner:** VentuSample  
**Status:** additive integration profile  
**Compatibility:** VentuSample remains an independent C++20/Qt 6 procedural sample workstation.

## Product role

VentuSample owns procedural sound generation, preset and pack metadata, WAV/MIDI workflows, sample-rate conversion and offline DSP. Its engine is scalar/offline by default. NEXUS does not rename the product, change its audio identity or claim it is a real-time DAW, VST/AU host or kernel audio driver.

## Integration model

- Exchange generated assets and metadata through the existing versioned VentuSample pack/preset schemas.
- Keep deterministic seed, variation, render settings, sample rate, channel layout and format metadata attached to generated outputs.
- Reaper may validate pack manifests and compare metadata read-only; it must not rewrite or reinterpret the audio payload.
- DuckOS may display pack and render metadata through a future user-space application.
- Pytyliki may provide independent bounded metrics where a genuine API and workload make that useful; do not create a direct dependency merely to increase integration count.

## NEXUS application consumer

**PulseLab** is a proposed companion application for offline render inspection. It may show waveform/peak/RMS views, render parameters, manifest validity, asset provenance and A/B comparisons. It must not imply real-time capture, hardware routing, low-latency playback or driver ownership.

**Cockos** and the existing VentuSample workbench remain unchanged. PulseLab is a separate companion tool, not a replacement for either application.

## Real-time audio is a separate project

A future real-time audio service requires an independent DuckOS design covering device ownership, sample-format negotiation, bounded ring buffers, callback/work scheduling, priority inversion, underrun recovery, clock drift and hardware-specific latency measurements. Offline DSP code may be reused only through an explicit real-time-safe API; allocation, locks, file I/O and unbounded work are prohibited in a real-time callback.

Do not publish a target such as end-to-end latency below 1 ms until reproducible tests identify hardware, driver/provider, buffer size, sample rate, scheduling mode and measured percentile distribution.

## Acceptance

- Offline render output remains deterministic for a fixed configuration and seed.
- Pack/preset schemas validate and reject oversized or malformed assets.
- NEXUS consumers treat imported assets as data, not executable plugins.
- No real-time or kernel capability is inferred from the existence of offline DSP.
