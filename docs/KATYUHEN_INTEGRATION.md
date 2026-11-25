# Katyuhen integration — VentuSample

## Role in the ecosystem

VentuSample is Katyuhen's audio workstation. It owns procedural sample-pack generation, offline WAV processing, DSP buffers, deterministic presets, spectrum analysis, MIDI inspection and the Qt desktop experience.

## Registered interfaces

| Interface | Direction | State | Boundary |
|---|---|---|---|
| VentuSample pack v1/v2 | Provides | Implemented | Versioned metadata and local WAV assets |
| Reaper C ABI v1 | Optional consumer | Partial | Explicit local checkout and build-time opt-in |
| Pytyliki C ABI v1 | Optional consumer | Partial | Explicit trusted library path for optional cross-checks |

## Integration rules

- Keep Reaper's expression evaluation off the real-time DSP path.
- Keep pack-v1 metadata auditing separate from waveform comparison or subjective quality scoring.
- Never auto-download, PATH-search or implicitly load a native dependency.
- Version pack schemas independently; v2 must not be silently interpreted as v1.
- Preserve standalone generation and offline processing without Reaper or Pytyliki.

## Workspace and visual identity

Mora is the ecosystem's desktop entry point, not a replacement for VentuSample Studio. The Qt workstation retains its specialist waveform and DSP workflows while native adapters may locally map semantic roles to DAK's `mora.theme.v1` tokens.

The Katyuhen registry is maintained in [DAK](https://github.com/Ivan-VHS/DAK/blob/main/ecosystem/katyuhen.v1.json). The local declaration is [integrations/katyuhen/manifest.json](../integrations/katyuhen/manifest.json).

## Validation

Run VentuSample's C++20 CI, Qt desktop jobs and offline DSP tests. Build-time optional bridges need their own test matrix.
