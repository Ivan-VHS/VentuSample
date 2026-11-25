# Reaper offline-render policy

**Owner:** VentuSample  
**Policy language:** Reaper  
**Status:** additive validation example

VentuSample remains a C++20/Qt 6 procedural sample workstation with offline DSP. The Reaper example [examples/reaper_offline_render_policy.rpr](../examples/reaper_offline_render_policy.rpr) validates a caller-supplied render descriptor and pack metadata before a separate host workflow decides whether to proceed.

## Enforced descriptor rules

- Sample rate is limited to the documented 48–96 kHz profile.
- Output is explicitly PCM16 with one or two channels.
- Frame count and payload size are bounded.
- Wet amount remains within the product's documented 65% ceiling.
- The manifest seed must match the render seed.
- Offline mode is required; realtime execution and manifest-driven plugin code execution are rejected.

The sample does not render audio, write files, open a device, load plugins or claim a real-time audio engine. Its values are synthetic contract fixtures, not measurements.

## Ownership

C++20 remains authoritative for audio generation, DSP, file formats and the desktop application. Reaper is an optional policy/conformance layer; it is not linked as a mandatory runtime by this example. Plugin discovery and reviewed handler registration continue to follow the existing [plugin platform](PLUGINS.md).

## Verification boundary

The Reaper source uses the current .rpr reference grammar. It is not advertised as part of the currently limited native AOT subset. A future native policy execution target must pass the same fixtures before it can replace the existing reference-toolchain path.
