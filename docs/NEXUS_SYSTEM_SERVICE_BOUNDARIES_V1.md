# VentuSample — NEXUS System-Service Boundaries v1

**Owner:** VentuSample  
**NEXUS role:** offline audio processing and asset provider  
**Status:** additive federation contract

VentuSample remains an independent C++20/Qt 6 procedural sample workstation with offline rendering and DSP. NEXUS does not rename the application, move its DSP into the kernel or imply a real-time audio device service.

## AudioSubsys relationship

AudioSubsys is a DAK-owned future device/session service. VentuSample may provide offline render jobs, generated assets and versioned pack/preset metadata through explicit user-space interfaces. It does not implement AudioSubsys or own DAK audio devices.

Permitted consumers:

- PulseLab may inspect offline render parameters, peak/RMS data, waveform summaries, pack manifests and asset provenance.
- DAK may display validated render metadata through an application surface after its user-space integration contract exists.
- Reaper may validate supported manifest metadata; it must not rewrite or reinterpret audio payloads.
- Pytyliki may supply independent bounded metrics only where a useful workload-specific interface exists. Do not add a dependency solely to increase federation count.

## Runtime and data boundaries

1. Keep the C++20 renderer, audio buffers, plugin lifecycle and offline export owned by VentuSample.
2. Use existing versioned pack/preset schemas and preserve seed, variation, sample rate, channel layout, render settings and source provenance.
3. Treat imported plugins and packs as data until a separately reviewed trust and execution model exists.
4. Offline render work must not be routed into a real-time callback by implication.
5. No hidden device access, microphone capture, background recording or cloud upload is introduced by NEXUS.
6. Existing Reaper expression integration remains optional control infrastructure outside the DSP path.

## Real-time audio is a separate OS capability

A future DAK real-time audio service requires its own design for device discovery and ownership, format negotiation, bounded ring buffers, callback scheduling, priority-inversion analysis, underrun recovery, clock drift and hardware-specific latency measurements. Allocation, locks, filesystem I/O and unbounded work are prohibited in a real-time callback.

Do not claim sub-millisecond latency or hardware support until reproducible tests name the device, driver/provider, sample rate, buffer size, scheduling policy and measured latency distribution.

## Acceptance

- Fixed configuration and seed produce deterministic offline output within the project's declared numeric tolerance.
- Pack/preset schemas reject malformed or oversized input.
- NEXUS consumers handle unavailable providers and preserve provenance.
- No real-time, kernel or device capability is inferred from the offline DSP implementation.

This profile integrates VentuSample as an offline audio provider only. It does not implement AudioSubsys.
